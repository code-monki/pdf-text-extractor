export type TaskStatus = "done" | "in-progress" | "blocked" | "deferred";

export interface WbsTask {
  id: string;
  name: string;
  status: TaskStatus;
  owner?: string;
  notes?: string;
  children?: WbsTask[];
}

export interface WbsDocument {
  meta: {
    title: string;
    sourceDocs: string[];
    lastReviewed: string;
    criticalPathHint: string;
  };
  roots: WbsTask[];
}

export interface RtmSummary {
  source: string;
  lastReviewed: string;
  note?: string;
  functionalRequirements: {
    count: number;
    idPrefix: string;
    idStart: number;
    idEnd: number;
    implementationStatus: TaskStatus;
    owner?: string;
  };
  nonFunctionalRequirements: {
    count: number;
    idPrefix: string;
    idStart: number;
    idEnd: number;
    implementationStatus: TaskStatus;
    owner?: string;
  };
  packagingColumnNote?: string;
}
