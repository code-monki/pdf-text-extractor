#!/usr/bin/env node

const { spawn } = require('child_process');
const fs = require('fs');
const os = require('os');
const path = require('path');

function parseCsv(value) {
  return value.split(',').map((item) => item.trim()).filter(Boolean);
}

function parseArgs(argv) {
  const args = {
    workDir: 'work/ct-b01-characters-combat-1981',
    pages: [],
    modes: ['3', '4', '6', '11'],
    jobs: Math.max(1, Math.min((os.availableParallelism?.() || os.cpus().length || 4) - 2, 8)),
    outputName: 'ocr-psm-comparison',
  };

  for (let i = 2; i < argv.length; i += 1) {
    const arg = argv[i];
    if (arg === '--work-dir') {
      args.workDir = argv[++i];
    } else if (arg === '--pages') {
      args.pages = parseCsv(argv[++i]);
    } else if (arg === '--modes') {
      args.modes = parseCsv(argv[++i]);
    } else if (arg === '--jobs') {
      args.jobs = Number(argv[++i]);
    } else if (arg === '--output-name') {
      args.outputName = argv[++i];
    } else if (arg === '--help') {
      printHelp();
      process.exit(0);
    } else {
      throw new Error(`Unknown argument: ${arg}`);
    }
  }

  if (args.pages.length === 0) {
    throw new Error('At least one page is required. Use --pages 0005,0015');
  }
  if (!Number.isInteger(args.jobs) || args.jobs < 1) {
    throw new Error('--jobs must be a positive integer');
  }
  if (!/^[a-zA-Z0-9._-]+$/.test(args.outputName)) {
    throw new Error('--output-name may contain only letters, numbers, dot, underscore, and dash');
  }

  args.pages = args.pages.map((page) => page.padStart(4, '0'));
  return args;
}

function printHelp() {
  console.log(`Usage:
  node scripts/experiments/compare-tesseract-psm.js \\
    --work-dir work/ct-b01-characters-combat-1981 \\
    --pages 0005,0015,0047 \\
    --modes 3,4,6,11 \\
    --jobs 8 \\
    --output-name ocr-psm-comparison

Writes ignored OCR candidates and diagnostics under the work directory.
Does not print OCR text.`);
}

function imagePathForPage(workDir, page) {
  const pageNumber = String(Number(page));
  const candidates = [
    path.join(workDir, 'raw', 'page-images', `page-${pageNumber}.png`),
    path.join(workDir, 'raw', 'page-images', `page-${pageNumber.padStart(2, '0')}.png`),
    path.join(workDir, 'raw', 'page-images', `page-${pageNumber.padStart(3, '0')}.png`),
    path.join(workDir, 'raw', 'page-images', `page-${pageNumber.padStart(4, '0')}.png`),
  ];
  return candidates.find((candidate) => fs.existsSync(candidate)) || candidates[0];
}

function outputBaseFor(workDir, page, mode) {
  return path.join(workDir, 'raw', 'ocr-psm', page, `psm-${mode}`);
}

function ensureDir(dir) {
  fs.mkdirSync(dir, { recursive: true });
}

function runTesseract(imagePath, outputBase, mode) {
  ensureDir(path.dirname(outputBase));
  const started = Date.now();
  return new Promise((resolve) => {
    const child = spawn('tesseract', [imagePath, outputBase, '--psm', mode, 'txt', 'tsv'], {
      stdio: ['ignore', 'pipe', 'pipe'],
    });
    let stdout = '';
    let stderr = '';
    child.stdout.setEncoding('utf8');
    child.stderr.setEncoding('utf8');
    child.stdout.on('data', (chunk) => { stdout += chunk; });
    child.stderr.on('data', (chunk) => { stderr += chunk; });
    child.on('error', (error) => {
      resolve({
        status: 1,
        elapsedMs: Date.now() - started,
        stderr: String(error.stack || error.message || error),
        stdout,
      });
    });
    child.on('close', (status) => {
      resolve({
        status,
        elapsedMs: Date.now() - started,
        stderr,
        stdout,
      });
    });
  });
}

async function runPool(tasks, jobs) {
  const rows = [];
  const errors = [];
  let next = 0;

  async function worker() {
    while (next < tasks.length) {
      const task = tasks[next];
      next += 1;
      const { page, mode, imagePath, outputBase } = task;
      const run = await runTesseract(imagePath, outputBase, mode);
      const textPath = `${outputBase}.txt`;
      const tsvPath = `${outputBase}.tsv`;
      const text = fs.existsSync(textPath) ? fs.readFileSync(textPath, 'utf8') : '';
      const metrics = metricsFor(text);
      rows.push({
        page,
        mode,
        status: run.status,
        elapsedMs: run.elapsedMs,
        ...metrics,
        ...confidenceMetricsFor(tsvPath),
        stderrChars: run.stderr.length,
      });
      if (run.status !== 0) {
        errors.push({ page, mode, status: run.status, stderr: run.stderr });
      }
    }
  }

  await Promise.all(Array.from({ length: Math.min(jobs, tasks.length) }, () => worker()));
  rows.sort((a, b) => a.page.localeCompare(b.page) || Number(a.mode) - Number(b.mode));
  return { rows, errors };
}

function countMatches(text, regex) {
  return [...text.matchAll(regex)].length;
}

function metricsFor(text) {
  const chars = text.length;
  const letters = countMatches(text, /\p{L}/gu);
  const digits = countMatches(text, /\p{Nd}/gu);
  const symbols = countMatches(text, /[^\p{L}\p{Nd}\s]/gu);
  const controls = countMatches(text, /[\x00-\x08\x0B\x0C\x0E-\x1F\x7F]/g);
  const words = countMatches(text, /\p{L}[\p{L}\p{Mn}'-]*/gu);
  const lines = text.split(/\n/);
  const nonEmptyLines = lines.filter((line) => line.trim().length > 0);
  const longLines = nonEmptyLines.filter((line) => line.length > 120).length;
  const maxLineLength = nonEmptyLines.reduce((max, line) => Math.max(max, line.length), 0);
  const symbolRatio = chars === 0 ? 0 : symbols / chars;
  const letterRatio = chars === 0 ? 0 : letters / chars;

  const flags = [];
  if (chars === 0) flags.push('empty');
  if (chars > 0 && chars < 100) flags.push('very-short');
  if (symbolRatio > 0.20) flags.push('high-symbol-ratio');
  if (chars > 0 && digits / chars > 0.35) flags.push('high-numeric-ratio');
  if (controls > 0) flags.push('unicode-suspect');
  if (longLines > 5) flags.push('many-long-lines');

  const simpleScore = (
    letters
    + words * 2
    - symbols * 3
    - controls * 20
    - longLines * 25
  );

  return {
    chars,
    letters,
    digits,
    symbols,
    controls,
    words,
    lines: lines.length,
    nonEmptyLines: nonEmptyLines.length,
    maxLineLength,
    longLines,
    symbolRatio,
    letterRatio,
    simpleScore,
    flags,
  };
}

function confidenceMetricsFor(tsvPath) {
  const empty = {
    confCount: 0,
    confMean: null,
    confMin: null,
    confLowCount: 0,
  };
  if (!fs.existsSync(tsvPath)) return empty;

  const content = fs.readFileSync(tsvPath, 'utf8').trimEnd();
  if (!content) return empty;

  const lines = content.split(/\n/);
  const header = lines.shift().split(/\t/);
  const confIndex = header.indexOf('conf');
  if (confIndex === -1) return empty;

  const values = [];
  for (const line of lines) {
    const cols = line.split(/\t/);
    const value = Number(cols[confIndex]);
    if (Number.isFinite(value) && value >= 0) values.push(value);
  }
  if (values.length === 0) return empty;

  const sum = values.reduce((total, value) => total + value, 0);
  return {
    confCount: values.length,
    confMean: sum / values.length,
    confMin: Math.min(...values),
    confLowCount: values.filter((value) => value < 60).length,
  };
}

function writeTsv(rows, filePath) {
  const header = [
    'page',
    'mode',
    'status',
    'elapsedMs',
    'chars',
    'letters',
    'digits',
    'symbols',
    'controls',
    'words',
    'lines',
    'nonEmptyLines',
    'maxLineLength',
    'longLines',
    'symbolRatio',
    'letterRatio',
    'simpleScore',
    'confCount',
    'confMean',
    'confMin',
    'confLowCount',
    'flags',
    'stderrChars',
  ];
  const lines = [header.join('\t')];
  for (const row of rows) {
    lines.push(header.map((key) => {
      const value = row[key];
      if (Array.isArray(value)) return value.join(',');
      if (typeof value === 'number') {
        return Number.isInteger(value) ? String(value) : value.toFixed(4);
      }
      return String(value ?? '');
    }).join('\t'));
  }
  fs.writeFileSync(filePath, `${lines.join('\n')}\n`);
}

async function main() {
  const args = parseArgs(process.argv);
  const diagnosticsDir = path.join(args.workDir, 'diagnostics');
  ensureDir(diagnosticsDir);

  const errors = [];
  const tasks = [];

  for (const page of args.pages) {
    const imagePath = imagePathForPage(args.workDir, page);
    if (!fs.existsSync(imagePath)) {
      errors.push({ page, error: `Missing image: ${imagePath}` });
      continue;
    }

    for (const mode of args.modes) {
      const outputBase = outputBaseFor(args.workDir, page, mode);
      tasks.push({ page, mode, imagePath, outputBase });
    }
  }

  const started = Date.now();
  const result = await runPool(tasks, args.jobs);
  const rows = result.rows;
  errors.push(...result.errors);
  const wallElapsedMs = Date.now() - started;

  const tsvPath = path.join(diagnosticsDir, `${args.outputName}.tsv`);
  const jsonPath = path.join(diagnosticsDir, `${args.outputName}-summary.json`);
  writeTsv(rows, tsvPath);

  const byPage = {};
  for (const row of rows) {
    byPage[row.page] ??= [];
    byPage[row.page].push({
      mode: row.mode,
      chars: row.chars,
      words: row.words,
      symbolRatio: row.symbolRatio,
      longLines: row.longLines,
      simpleScore: row.simpleScore,
      confCount: row.confCount,
      confMean: row.confMean,
      confMin: row.confMin,
      confLowCount: row.confLowCount,
      flags: row.flags,
      elapsedMs: row.elapsedMs,
    });
  }

  const selectedBySimpleScore = Object.fromEntries(Object.entries(byPage).map(([page, pageRows]) => {
    const selected = [...pageRows].sort((a, b) => b.simpleScore - a.simpleScore)[0];
    return [page, selected.mode];
  }));

  fs.writeFileSync(jsonPath, `${JSON.stringify({
    generatedAt: new Date().toISOString(),
    workDir: args.workDir,
    pages: args.pages,
    modes: args.modes,
    jobs: args.jobs,
    outputName: args.outputName,
    wallElapsedMs,
    selectedBySimpleScore,
    errors,
    note: 'Exploratory metrics only. Does not establish final OCR candidate selection logic.',
  }, null, 2)}\n`);

  console.log(`rows=${rows.length}`);
  console.log(`pages=${args.pages.length}`);
  console.log(`modes=${args.modes.join(',')}`);
  console.log(`jobs=${args.jobs}`);
  console.log(`wallElapsedMs=${wallElapsedMs}`);
  console.log(`tsv=${tsvPath}`);
  console.log(`summary=${jsonPath}`);
  console.log(`errors=${errors.length}`);
}

try {
  main().catch((error) => {
    console.error(error.message);
    process.exit(1);
  });
} catch (error) {
  console.error(error.message);
  process.exit(1);
}
