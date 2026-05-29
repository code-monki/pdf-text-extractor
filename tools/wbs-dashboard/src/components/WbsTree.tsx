import { useState } from "react";
import type { TaskStatus, WbsTask } from "../types";
import { statusLabel } from "../wbsUtils";
import styles from "./WbsTree.module.css";

interface Props {
  roots: WbsTask[];
}

function badgeClass(s: TaskStatus): string {
  switch (s) {
    case "done":
      return styles.badgeDone;
    case "in-progress":
      return styles.badgeInProgress;
    case "blocked":
      return styles.badgeBlocked;
    case "deferred":
      return styles.badgeDeferred;
    default:
      return "";
  }
}

function NodeRow({ task, depth }: { task: WbsTask; depth: number }) {
  const [open, setOpen] = useState(depth < 2);
  const hasKids = Boolean(task.children?.length);

  return (
    <li className={styles.node}>
      <div className={styles.nodeInner}>
        <div className={styles.row}>
          {hasKids ? (
            <button
              type="button"
              className={styles.toggle}
              aria-expanded={open}
              onClick={() => setOpen(!open)}
            >
              {open ? "▼" : "▶"}
            </button>
          ) : (
            <span className={styles.toggleSpacer} aria-hidden />
          )}
          <div className={styles.body}>
            <div className={styles.name}>{task.name}</div>
            <div className={styles.meta}>
              <span className={`${styles.badge} ${badgeClass(task.status)}`}>
                {statusLabel(task.status)}
              </span>
              {task.owner && <span>Owner: {task.owner}</span>}
              <span className={styles.id}>#{task.id}</span>
            </div>
            {task.notes && <div className={styles.notes}>{task.notes}</div>}
          </div>
        </div>
        {hasKids && open && (
          <ul className={styles.children}>
            {task.children!.map((c) => (
              <NodeRow key={c.id} task={c} depth={depth + 1} />
            ))}
          </ul>
        )}
      </div>
    </li>
  );
}

export function WbsTree({ roots }: Props) {
  if (roots.length === 0) {
    return (
      <div className={styles.tree}>
        <p className={styles.empty}>No tasks match the current filters.</p>
      </div>
    );
  }

  return (
    <div className={styles.tree}>
      <ul className={styles.node}>
        {roots.map((t) => (
          <NodeRow key={t.id} task={t} depth={0} />
        ))}
      </ul>
    </div>
  );
}
