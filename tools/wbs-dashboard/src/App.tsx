import { useEffect, useMemo, useState } from "react";
import type { TaskStatus, WbsDocument, RtmSummary } from "./types";
import { collectOwners, countByStatus, filterTree, flattenTasks } from "./wbsUtils";
import { FiltersBar } from "./components/FiltersBar";
import { SummaryCards } from "./components/SummaryCards";
import { WbsTree } from "./components/WbsTree";
import { RtmPanel } from "./components/RtmPanel";
import styles from "./App.module.css";

type Tab = "wbs" | "rtm";

const DEFAULT_STATUSES: TaskStatus[] = [
  "done",
  "in-progress",
  "blocked",
  "deferred",
];

export default function App() {
  const [tab, setTab] = useState<Tab>("wbs");
  const [wbs, setWbs] = useState<WbsDocument | null>(null);
  const [rtm, setRtm] = useState<RtmSummary | null>(null);
  const [err, setErr] = useState<string | null>(null);

  const [statusFilter, setStatusFilter] = useState<Set<TaskStatus>>(
    () => new Set(DEFAULT_STATUSES),
  );
  const [ownerFilter, setOwnerFilter] = useState<string | null>(null);

  useEffect(() => {
    let cancelled = false;
    (async () => {
      try {
        const [wbsRes, rtmRes] = await Promise.all([
          fetch("/wbs-data.json"),
          fetch("/rtm-summary.json"),
        ]);
        if (!wbsRes.ok) throw new Error(`wbs-data.json: ${wbsRes.status}`);
        if (!rtmRes.ok) throw new Error(`rtm-summary.json: ${rtmRes.status}`);
        const wbsJson = (await wbsRes.json()) as WbsDocument;
        const rtmJson = (await rtmRes.json()) as RtmSummary;
        if (!cancelled) {
          setWbs(wbsJson);
          setRtm(rtmJson);
        }
      } catch (e) {
        if (!cancelled) {
          setErr(e instanceof Error ? e.message : String(e));
        }
      }
    })();
    return () => {
      cancelled = true;
    };
  }, []);

  const owners = useMemo(() => {
    if (!wbs) return [];
    return collectOwners(wbs.roots);
  }, [wbs]);

  const filteredRoots = useMemo(() => {
    if (!wbs) return [];
    const status =
      statusFilter.size === 0 || statusFilter.size === DEFAULT_STATUSES.length
        ? null
        : statusFilter;
    return filterTree(wbs.roots, status, ownerFilter);
  }, [wbs, statusFilter, ownerFilter]);

  const summaryCounts = useMemo(() => {
    if (!wbs) {
      return {
        done: 0,
        "in-progress": 0,
        blocked: 0,
        deferred: 0,
      } as Record<TaskStatus, number>;
    }
    return countByStatus(flattenTasks(filteredRoots));
  }, [wbs, filteredRoots]);

  if (err) {
    return (
      <div className={styles.root}>
        <div className={styles.shell}>
          <p className={styles.sub}>Failed to load data: {err}</p>
        </div>
      </div>
    );
  }

  if (!wbs || !rtm) {
    return (
      <div className={styles.root}>
        <div className={styles.shell}>
          <p className={styles.sub}>Loading…</p>
        </div>
      </div>
    );
  }

  return (
    <div className={styles.root}>
      <div className={styles.shell}>
        <header className={styles.header}>
          <h1>{wbs.meta.title}</h1>
          <p className={styles.sub}>
            Offline-first WBS view — data from checked-in JSON under{" "}
            <code>public/</code>.
          </p>
          <div className={styles.sources}>
            <span>Sources (edit JSON when docs change):</span>
            <ul>
              {wbs.meta.sourceDocs.map((s) => (
                <li key={s}>{s}</li>
              ))}
            </ul>
          </div>
        </header>

        <div className={styles.tabs}>
          <button
            type="button"
            className={tab === "wbs" ? `${styles.tab} ${styles.tabActive}` : styles.tab}
            onClick={() => setTab("wbs")}
          >
            WBS
          </button>
          <button
            type="button"
            className={tab === "rtm" ? `${styles.tab} ${styles.tabActive}` : styles.tab}
            onClick={() => setTab("rtm")}
          >
            RTM summary
          </button>
        </div>

        {tab === "wbs" ? (
          <>
            <SummaryCards counts={summaryCounts} />
            <FiltersBar
              statusFilter={statusFilter}
              onStatusChange={setStatusFilter}
              ownerFilter={ownerFilter}
              onOwnerChange={setOwnerFilter}
              owners={owners}
            />
            <WbsTree roots={filteredRoots} />
            <div className={styles.critical}>
              <strong>Critical path (from data):</strong> {wbs.meta.criticalPathHint}
            </div>
          </>
        ) : (
          <RtmPanel data={rtm} />
        )}
      </div>
    </div>
  );
}
