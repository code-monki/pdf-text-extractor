#!/usr/bin/env python3
"""
Apply derived outline/link enrichment to a PDF using pikepdf.

This script never edits source in place; caller provides distinct output path.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import pikepdf


def _load_json(path: Path):
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def _parse_destination(pdf: pikepdf.Pdf, destination: dict):
    page_index = int(destination.get("pageIndex", 0))
    y_pt = float(destination.get("yPt", 0.0))
    if page_index < 0 or page_index >= len(pdf.pages):
        raise ValueError(f"invalid destination page index: {page_index}")
    page_obj = pdf.pages[page_index].obj
    # [page /XYZ left top zoom]
    return pikepdf.Array([page_obj, pikepdf.Name("/XYZ"), 0, y_pt, 0])


def _apply_outline(pdf: pikepdf.Pdf, outline_map: dict):
    nodes = outline_map.get("nodes", [])
    stack = []
    destination_registry = {}
    with pdf.open_outline() as outline:
        for node in nodes:
            level = int(node.get("level", 1))
            title = str(node.get("title", "Untitled"))
            dest = _parse_destination(pdf, node.get("destination", {}))
            node_id = node.get("id")
            if isinstance(node_id, str) and node_id:
                destination_registry[node_id] = dest
            item = pikepdf.OutlineItem(title, dest)
            if level <= 1 or not stack:
                outline.root.append(item)
                stack = [item]
                continue
            parent_level = min(level - 1, len(stack))
            parent = stack[parent_level - 1]
            parent.children.append(item)
            stack = stack[:parent_level] + [item]
    return destination_registry


def _make_annotation(pdf: pikepdf.Pdf, link: dict, destination_registry: dict):
    rect = link.get("rect")
    if not isinstance(rect, list) or len(rect) != 4:
        raise ValueError("rect must contain four numbers")
    rect_array = pikepdf.Array([float(rect[0]), float(rect[1]), float(rect[2]), float(rect[3])])
    target = link.get("target", {})
    target_type = target.get("type")

    annot = pikepdf.Dictionary(
        Type=pikepdf.Name("/Annot"),
        Subtype=pikepdf.Name("/Link"),
        Rect=rect_array,
        Border=pikepdf.Array([0, 0, 0]),
    )

    if target_type == "url":
        url = str(target.get("url", ""))
        if not (url.startswith("https://") or url.startswith("http://")):
            raise ValueError("url target must start with http:// or https://")
        annot["/A"] = pikepdf.Dictionary(
            S=pikepdf.Name("/URI"),
            URI=url,
        )
    elif target_type == "inter":
        file_name = str(target.get("path") or target.get("volumeId", ""))
        dest_name = str(target.get("destinationId", ""))
        if not file_name or not dest_name:
            raise ValueError("inter target requires path/volumeId and destinationId")
        annot["/A"] = pikepdf.Dictionary(
            S=pikepdf.Name("/GoToR"),
            F=file_name,
            D=dest_name,
            NewWindow=True,
        )
    else:
        # Default: in-document link destination
        dest_obj = target.get("destination")
        if isinstance(dest_obj, dict):
            annot["/Dest"] = _parse_destination(pdf, dest_obj)
        else:
            dest_id = str(target.get("destinationId", ""))
            if not dest_id:
                raise ValueError("intra target requires destination or destinationId")
            if dest_id not in destination_registry:
                raise ValueError(f"intra destinationId not found: {dest_id}")
            annot["/Dest"] = destination_registry[dest_id]
    return annot


def _apply_links(pdf: pikepdf.Pdf, link_map: dict, destination_registry: dict):
    links = link_map.get("links", [])
    for link in links:
        page_index = int(link.get("pageIndex", 0))
        if page_index < 0 or page_index >= len(pdf.pages):
            raise ValueError(f"invalid link page index: {page_index}")
        page = pdf.pages[page_index]
        annots = page.obj.get("/Annots")
        if annots is None:
            annots = pikepdf.Array()
            page.obj["/Annots"] = annots
        annot = _make_annotation(pdf, link, destination_registry)
        annots.append(pdf.make_indirect(annot))


def main():
    parser = argparse.ArgumentParser(description="Apply outline/link enrichment to derived PDF.")
    parser.add_argument("--source", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--outline-map")
    parser.add_argument("--link-map")
    args = parser.parse_args()

    source = Path(args.source)
    output = Path(args.output)
    if source.resolve() == output.resolve():
        raise SystemExit("source and output paths must differ")
    output.parent.mkdir(parents=True, exist_ok=True)

    with pikepdf.open(str(source)) as pdf:
        destination_registry = {}
        if args.outline_map:
            destination_registry = _apply_outline(pdf, _load_json(Path(args.outline_map)))
        if args.link_map:
            _apply_links(pdf, _load_json(Path(args.link_map)), destination_registry)
        pdf.save(str(output))

    print("ok")


if __name__ == "__main__":
    main()

