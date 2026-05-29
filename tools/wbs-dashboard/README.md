# WBS dashboard (local)

Offline-first **work breakdown** and **RTM summary** view for `pdf-text-extractor`. Data is **checked-in JSON** under `public/` — no network or PDF corpus required.

## When to edit data

- After **`docs/phase-gate-record.md`** changes (gates, Gate 9 open work, deferrals), update **`public/wbs-data.json`** so the tree matches governance.
- After **`docs/requirements-traceability-matrix.md`** requirement rows change, update **`public/rtm-summary.json`** (counts and ID ranges).

## Commands

```bash
cd tools/wbs-dashboard
npm install
npm run dev
```

Build (typecheck + production bundle):

```bash
npm run build
```

Preview the production build:

```bash
npm run preview
```

## Files

| Path | Role |
|------|------|
| `public/wbs-data.json` | WBS tree, meta, critical-path hint |
| `public/rtm-summary.json` | FR/NFR counts and ID ranges (trimmed) |
| `src/` | React UI |
