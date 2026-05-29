import type { TaskStatus, WbsTask } from "./types";

export function flattenTasks(nodes: WbsTask[]): WbsTask[] {
  const out: WbsTask[] = [];
  const walk = (n: WbsTask) => {
    out.push(n);
    n.children?.forEach(walk);
  };
  nodes.forEach(walk);
  return out;
}

export function countByStatus(nodes: WbsTask[]): Record<TaskStatus, number> {
  const flat = flattenTasks(nodes);
  const init: Record<TaskStatus, number> = {
    done: 0,
    "in-progress": 0,
    blocked: 0,
    deferred: 0,
  };
  for (const t of flat) {
    init[t.status] += 1;
  }
  return init;
}

export function collectOwners(nodes: WbsTask[]): string[] {
  const s = new Set<string>();
  for (const t of flattenTasks(nodes)) {
    if (t.owner) s.add(t.owner);
  }
  return [...s].sort((a, b) => a.localeCompare(b));
}

/** Include node if it or any descendant passes filters. */
export function filterTree(
  nodes: WbsTask[],
  status: Set<TaskStatus> | null,
  owner: string | null,
): WbsTask[] {
  const matchSelf = (t: WbsTask) => {
    const okStatus = !status || status.size === 0 || status.has(t.status);
    const okOwner = !owner || t.owner === owner;
    return okStatus && okOwner;
  };

  const filterNode = (t: WbsTask): WbsTask | null => {
    const childrenIn = t.children ?? [];
    const filteredKids = childrenIn
      .map(filterNode)
      .filter((c): c is WbsTask => c !== null);

    if (matchSelf(t)) {
      return {
        ...t,
        children:
          filteredKids.length > 0
            ? filteredKids
            : childrenIn.length > 0
              ? []
              : undefined,
      };
    }
    if (filteredKids.length > 0) {
      return { ...t, children: filteredKids };
    }
    return null;
  };

  return nodes
    .map(filterNode)
    .filter((c): c is WbsTask => c !== null);
}

export function nextAttentionHint(counts: Record<TaskStatus, number>): string {
  if (counts.blocked > 0) {
    return `${counts.blocked} blocked task(s) — resolve or unblock before release momentum.`;
  }
  if (counts["in-progress"] > 0) {
    return `${counts["in-progress"]} in-progress — close or re-scope before picking up deferred work.`;
  }
  if (counts.deferred > 0) {
    return "No blocked items; deferred backlog exists (Phase B / orchestration) — confirm authorization before scheduling.";
  }
  return "All tracked items are done for this snapshot — verify phase-gate record for authoritative gate status.";
}

export function statusLabel(s: TaskStatus): string {
  switch (s) {
    case "done":
      return "Done";
    case "in-progress":
      return "In progress";
    case "blocked":
      return "Blocked";
    case "deferred":
      return "Deferred";
    default:
      return s;
  }
}
