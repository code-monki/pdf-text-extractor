import type { TaskStatus } from "../types";
import { statusLabel } from "../wbsUtils";
import styles from "./FiltersBar.module.css";

const ALL_STATUSES: TaskStatus[] = [
  "done",
  "in-progress",
  "blocked",
  "deferred",
];

interface Props {
  statusFilter: Set<TaskStatus>;
  onStatusChange: (next: Set<TaskStatus>) => void;
  ownerFilter: string | null;
  onOwnerChange: (owner: string | null) => void;
  owners: string[];
}

export function FiltersBar({
  statusFilter,
  onStatusChange,
  ownerFilter,
  onOwnerChange,
  owners,
}: Props) {
  const toggle = (s: TaskStatus) => {
    const next = new Set(statusFilter);
    if (next.has(s)) next.delete(s);
    else next.add(s);
    onStatusChange(next);
  };

  return (
    <div className={styles.filters}>
      <div className={styles.group}>
        <span className={styles.label}>Status</span>
        <div className={styles.statusRow}>
          {ALL_STATUSES.map((s) => (
            <label key={s} className={styles.cb}>
              <input
                type="checkbox"
                checked={statusFilter.has(s)}
                onChange={() => toggle(s)}
              />
              {statusLabel(s)}
            </label>
          ))}
        </div>
      </div>
      <div className={styles.group}>
        <span className={styles.label}>Owner</span>
        <select
          className={styles.select}
          value={ownerFilter ?? ""}
          onChange={(e) =>
            onOwnerChange(e.target.value === "" ? null : e.target.value)
          }
        >
          <option value="">Any</option>
          {owners.map((o) => (
            <option key={o} value={o}>
              {o}
            </option>
          ))}
        </select>
      </div>
      <button
        type="button"
        className={styles.reset}
        onClick={() => {
          onStatusChange(new Set(ALL_STATUSES));
          onOwnerChange(null);
        }}
      >
        Reset filters
      </button>
    </div>
  );
}
