import type { RtmSummary, WbsTask } from "../types";
import { WbsTree } from "./WbsTree";
import styles from "./RtmPanel.module.css";

interface Props {
  data: RtmSummary;
}

function toSyntheticWbs(data: RtmSummary): WbsTask[] {
  const { functionalRequirements: fr, nonFunctionalRequirements: nfr } = data;
  const frLabel = `${fr.idPrefix}-${String(fr.idStart).padStart(3, "0")} – ${fr.idPrefix}-${String(fr.idEnd).padStart(3, "0")}`;
  const nfrLabel = `${nfr.idPrefix}-${String(nfr.idStart).padStart(3, "0")} – ${nfr.idPrefix}-${String(nfr.idEnd).padStart(3, "0")}`;

  return [
    {
      id: "rtm-root",
      name: `Requirements (${data.source})`,
      status: "done",
      owner: "RTM",
      children: [
        {
          id: "rtm-fr",
          name: `Functional requirements (${fr.count})`,
          status: fr.implementationStatus,
          owner: fr.owner,
          notes: `Synthesized leaf range ${frLabel} — verify after RTM table edits.`,
          children: [
            {
              id: "rtm-fr-leaf",
              name: frLabel,
              status: fr.implementationStatus,
              owner: fr.owner,
            },
          ],
        },
        {
          id: "rtm-nfr",
          name: `Non-functional requirements (${nfr.count})`,
          status: nfr.implementationStatus,
          owner: nfr.owner,
          notes: `Synthesized leaf range ${nfrLabel} — verify after RTM table edits.`,
          children: [
            {
              id: "rtm-nfr-leaf",
              name: nfrLabel,
              status: nfr.implementationStatus,
              owner: nfr.owner,
            },
          ],
        },
      ],
    },
  ];
}

export function RtmPanel({ data }: Props) {
  const synthetic = toSyntheticWbs(data);

  return (
    <div>
      <p className={styles.intro}>
        Static extract for planning visibility. Full traceability lives in{" "}
        <code>{data.source}</code>. Update{" "}
        <code>public/rtm-summary.json</code> when the RTM changes.
      </p>
      {data.packagingColumnNote && (
        <p className={styles.note}>{data.packagingColumnNote}</p>
      )}
      {data.note && <p className={styles.note}>{data.note}</p>}
      <div className={styles.counts}>
        <span>
          <strong>{data.functionalRequirements.count}</strong> FR
        </span>
        <span>
          <strong>{data.nonFunctionalRequirements.count}</strong> NFR
        </span>
        <span className={styles.meta}>Reviewed {data.lastReviewed}</span>
      </div>
      <WbsTree roots={synthetic} />
    </div>
  );
}
