import type { TaskStatus } from "../types";
import { nextAttentionHint, statusLabel } from "../wbsUtils";
import styles from "./SummaryCards.module.css";

interface Props {
  counts: Record<TaskStatus, number>;
}

const cardClass = (s: TaskStatus): string => {
  switch (s) {
    case "done":
      return styles.done;
    case "in-progress":
      return styles.inProgress;
    case "blocked":
      return styles.blocked;
    case "deferred":
      return styles.deferred;
    default:
      return "";
  }
};

const ORDER: TaskStatus[] = [
  "blocked",
  "in-progress",
  "done",
  "deferred",
];

export function SummaryCards({ counts }: Props) {
  return (
    <div className={styles.grid}>
      {ORDER.map((s) => (
        <div key={s} className={styles.card}>
          <div className={styles.cardLabel}>{statusLabel(s)}</div>
          <div className={`${styles.cardValue} ${cardClass(s)}`}>
            {counts[s]}
          </div>
        </div>
      ))}
      <div className={styles.hint}>
        <strong>Next focus:</strong> {nextAttentionHint(counts)}
      </div>
    </div>
  );
}
