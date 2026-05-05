#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

function parseCsv(value) {
  return value.split(',').map((item) => item.trim()).filter(Boolean);
}

function parseArgs(argv) {
  const args = {
    workDir: 'work/ct-b01-characters-combat-1981',
    pages: [],
    modes: ['embedded', 'ocr', 'ocrmypdf-force', 'ocrmypdf-force-deskew-clean', 'psm-1', 'psm-3', 'psm-4', 'psm-6', 'psm-11', 'psm-12'],
    outputName: 'ground-truth-comparison',
  };

  for (let i = 2; i < argv.length; i += 1) {
    const arg = argv[i];
    if (arg === '--work-dir') {
      args.workDir = argv[++i];
    } else if (arg === '--pages') {
      args.pages = parseCsv(argv[++i]).map((page) => page.padStart(4, '0'));
    } else if (arg === '--modes') {
      args.modes = parseCsv(argv[++i]);
    } else if (arg === '--output-name') {
      args.outputName = argv[++i];
    } else if (arg === '--help') {
      printHelp();
      process.exit(0);
    } else {
      throw new Error(`Unknown argument: ${arg}`);
    }
  }

  if (!/^[a-zA-Z0-9._-]+$/.test(args.outputName)) {
    throw new Error('--output-name may contain only letters, numbers, dot, underscore, and dash');
  }

  if (args.pages.length === 0) {
    const gtDir = path.join(args.workDir, 'ground-truth');
    args.pages = fs.readdirSync(gtDir)
      .filter((file) => /^\d{4}\.txt$/.test(file))
      .map((file) => file.slice(0, 4))
      .sort();
  }

  return args;
}

function printHelp() {
  console.log(`Usage:
  node scripts/experiments/compare-ocr-to-ground-truth.js \\
    --work-dir work/ct-b01-characters-combat-1981 \\
    --pages 0005,0015,0045,0047,0049 \\
    --modes embedded,ocr,psm-3,psm-4,psm-6,psm-11,psm-12

Writes aggregate metrics only. Does not print ground-truth or OCR text.`);
}

function candidatePath(workDir, page, mode) {
  if (mode === 'embedded') return path.join(workDir, 'raw', 'embedded', `${page}.txt`);
  if (mode === 'ocr') return path.join(workDir, 'raw', 'ocr', `${page}.txt`);
  if (mode === 'ocrmypdf-force') return path.join(workDir, 'raw', 'ocrmypdf', 'pages', `${page}.txt`);
  if (mode === 'ocrmypdf-force-deskew-clean') return path.join(workDir, 'raw', 'ocrmypdf', 'deskew-clean-pages', `${page}.txt`);
  if (/^psm-\d+$/.test(mode)) {
    return path.join(workDir, 'raw', 'ocr-psm', page, `${mode}.txt`);
  }
  throw new Error(`Unknown mode: ${mode}`);
}

function normalizeText(text) {
  return text
    .normalize('NFKC')
    .replace(/\r\n?/g, '\n')
    .replace(/[ \t]+/g, ' ')
    .replace(/[ \t]*\n[ \t]*/g, '\n')
    .trim();
}

function wordsFor(text) {
  return normalizeText(text)
    .toLowerCase()
    .match(/\p{L}[\p{L}\p{Mn}'-]*|\p{Nd}+/gu) || [];
}

function levenshtein(a, b) {
  const previous = Array.from({ length: b.length + 1 }, (_, index) => index);
  const current = Array.from({ length: b.length + 1 }, () => 0);

  for (let i = 1; i <= a.length; i += 1) {
    current[0] = i;
    for (let j = 1; j <= b.length; j += 1) {
      const cost = a[i - 1] === b[j - 1] ? 0 : 1;
      current[j] = Math.min(
        previous[j] + 1,
        current[j - 1] + 1,
        previous[j - 1] + cost,
      );
    }
    for (let j = 0; j <= b.length; j += 1) previous[j] = current[j];
  }

  return previous[b.length];
}

function wordEditDistance(refWords, hypWords) {
  return levenshtein(refWords, hypWords);
}

function charErrorRate(ref, hyp) {
  const refChars = [...normalizeText(ref)];
  const hypChars = [...normalizeText(hyp)];
  if (refChars.length === 0) return hypChars.length === 0 ? 0 : 1;
  return levenshtein(refChars, hypChars) / refChars.length;
}

function wordErrorRate(refWords, hypWords) {
  if (refWords.length === 0) return hypWords.length === 0 ? 0 : 1;
  return wordEditDistance(refWords, hypWords) / refWords.length;
}

function bagStats(refWords, hypWords) {
  const ref = new Map();
  const hyp = new Map();
  for (const word of refWords) ref.set(word, (ref.get(word) || 0) + 1);
  for (const word of hypWords) hyp.set(word, (hyp.get(word) || 0) + 1);

  let matched = 0;
  let missing = 0;
  let extra = 0;
  for (const [word, count] of ref.entries()) {
    const h = hyp.get(word) || 0;
    matched += Math.min(count, h);
    if (count > h) missing += count - h;
  }
  for (const [word, count] of hyp.entries()) {
    const r = ref.get(word) || 0;
    if (count > r) extra += count - r;
  }
  return { matched, missing, extra };
}

function writeTsv(rows, filePath) {
  const header = [
    'page',
    'mode',
    'candidateExists',
    'refChars',
    'candidateChars',
    'refWords',
    'candidateWords',
    'cer',
    'wer',
    'bagMatched',
    'bagMissing',
    'bagExtra',
  ];
  const lines = [header.join('\t')];
  for (const row of rows) {
    lines.push(header.map((key) => {
      const value = row[key];
      if (typeof value === 'number') {
        return Number.isInteger(value) ? String(value) : value.toFixed(6);
      }
      return String(value ?? '');
    }).join('\t'));
  }
  fs.writeFileSync(filePath, `${lines.join('\n')}\n`);
}

function main() {
  const args = parseArgs(process.argv);
  const diagnosticsDir = path.join(args.workDir, 'diagnostics');
  fs.mkdirSync(diagnosticsDir, { recursive: true });

  const rows = [];
  const errors = [];
  for (const page of args.pages) {
    const truthPath = path.join(args.workDir, 'ground-truth', `${page}.txt`);
    if (!fs.existsSync(truthPath)) {
      errors.push({ page, error: `Missing ground truth: ${truthPath}` });
      continue;
    }
    const truth = fs.readFileSync(truthPath, 'utf8');
    const refWords = wordsFor(truth);

    for (const mode of args.modes) {
      const cPath = candidatePath(args.workDir, page, mode);
      const candidateExists = fs.existsSync(cPath);
      const candidate = candidateExists ? fs.readFileSync(cPath, 'utf8') : '';
      const hypWords = wordsFor(candidate);
      const bag = bagStats(refWords, hypWords);
      rows.push({
        page,
        mode,
        candidateExists,
        refChars: normalizeText(truth).length,
        candidateChars: normalizeText(candidate).length,
        refWords: refWords.length,
        candidateWords: hypWords.length,
        cer: charErrorRate(truth, candidate),
        wer: wordErrorRate(refWords, hypWords),
        bagMatched: bag.matched,
        bagMissing: bag.missing,
        bagExtra: bag.extra,
      });
    }
  }

  const tsvPath = path.join(diagnosticsDir, `${args.outputName}.tsv`);
  const summaryPath = path.join(diagnosticsDir, `${args.outputName}-summary.json`);
  writeTsv(rows, tsvPath);

  const byMode = {};
  for (const row of rows) {
    byMode[row.mode] ??= { pages: 0, cer: 0, wer: 0, missing: 0, extra: 0 };
    byMode[row.mode].pages += 1;
    byMode[row.mode].cer += row.cer;
    byMode[row.mode].wer += row.wer;
    byMode[row.mode].missing += row.bagMissing;
    byMode[row.mode].extra += row.bagExtra;
  }
  for (const item of Object.values(byMode)) {
    item.meanCer = item.cer / item.pages;
    item.meanWer = item.wer / item.pages;
    delete item.cer;
    delete item.wer;
  }

  const bestByPageCer = {};
  const bestByPageWer = {};
  for (const page of args.pages) {
    const pageRows = rows.filter((row) => row.page === page && row.candidateExists);
    if (pageRows.length === 0) continue;
    bestByPageCer[page] = [...pageRows].sort((a, b) => a.cer - b.cer)[0].mode;
    bestByPageWer[page] = [...pageRows].sort((a, b) => a.wer - b.wer)[0].mode;
  }

  fs.writeFileSync(summaryPath, `${JSON.stringify({
    generatedAt: new Date().toISOString(),
    workDir: args.workDir,
    pages: args.pages,
    modes: args.modes,
    byMode,
    bestByPageCer,
    bestByPageWer,
    errors,
    note: 'Ground truth is local-only. Summary contains aggregate metrics only.',
  }, null, 2)}\n`);

  console.log(`rows=${rows.length}`);
  console.log(`pages=${args.pages.length}`);
  console.log(`modes=${args.modes.join(',')}`);
  console.log(`tsv=${tsvPath}`);
  console.log(`summary=${summaryPath}`);
  console.log(`errors=${errors.length}`);
}

try {
  main();
} catch (error) {
  console.error(error.message);
  process.exit(1);
}
