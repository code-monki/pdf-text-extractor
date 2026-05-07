#!/bin/sh
# Pre-flight: validate enrichment sidecars for many volumes (pte_enrich --lint-only).
# Requirement refs: FR-034, FR-035, FR-036; NFR-014 (packaging handoff).
#
# Usage:
#   PTE_ENRICH=/path/to/pte_enrich scripts/enrich_lint_manifest.sh MANIFEST.tsv
#
# MANIFEST.tsv: one row per volume, UTF-8, tab-separated:
#   source_pdf<TAB>outline_map<TAB>link_map
# Optional columns may be empty; at least one of outline_map or link_map must be non-empty per row.
# Lines starting with # and blank lines are ignored.
# Paths may be absolute or relative to the repository root (parent of tests/).

set -e
set -u

trim_cr() {
	printf '%s' "$1" | tr -d '\r'
}

repo_root=$(CDPATH= cd "$(dirname "$0")/.." && pwd)
pte_enrich=${PTE_ENRICH:-"$repo_root/build/pte_enrich"}

if [ ! -x "$pte_enrich" ] && command -v pte_enrich >/dev/null 2>&1; then
	pte_enrich=$(command -v pte_enrich)
fi

if [ $# -lt 1 ]; then
	printf '%s\n' "usage: PTE_ENRICH=path/to/pte_enrich $0 MANIFEST.tsv" >&2
	exit 2
fi

manifest=$1
if [ ! -f "$manifest" ]; then
	printf '%s\n' "manifest not found: $manifest" >&2
	exit 2
fi

if [ ! -x "$pte_enrich" ]; then
	printf '%s\n' "pte_enrich not executable (set PTE_ENRICH): $pte_enrich" >&2
	exit 2
fi

resolve() {
	_path=$1
	case $_path in
	/*) printf '%s\n' "$_path" ;;
	*) printf '%s\n' "$repo_root/$_path" ;;
	esac
}

line_no=0
while IFS=$(printf '\t') read -r source_pdf outline_map link_map; do
	line_no=$((line_no + 1))

	source_pdf=$(trim_cr "$source_pdf")
	outline_map=$(trim_cr "${outline_map:-}")
	link_map=$(trim_cr "${link_map:-}")

	case $source_pdf in
	''|\#*) continue ;;
	esac

	if [ -z "$outline_map" ] && [ -z "$link_map" ]; then
		printf '%s\n' "$manifest:$line_no: need outline_map and/or link_map for source $source_pdf" >&2
		exit 1
	fi

	src_path=$(resolve "$source_pdf")

	set -- "$pte_enrich" --source "$src_path" --lint-only
	if [ -n "$outline_map" ]; then
		set -- "$@" --outline-map "$(resolve "$outline_map")"
	fi
	if [ -n "$link_map" ]; then
		set -- "$@" --link-map "$(resolve "$link_map")"
	fi

	printf '%s\n' "lint $line_no: $src_path"
	"$@"
done < "$manifest"

printf '%s\n' "enrich manifest lint: ok ($manifest)"
