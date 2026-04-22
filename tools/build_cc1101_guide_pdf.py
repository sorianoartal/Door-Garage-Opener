from __future__ import annotations

import html
import re
import sys
from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import mm
from reportlab.platypus import Paragraph, SimpleDocTemplate, Spacer, XPreformatted


INLINE_CODE_RE = re.compile(r"`([^`]+)`")
NUMBERED_RE = re.compile(r"^(\d+)\.\s+(.*)$")


def style_inline_code(text: str) -> str:
    escaped = html.escape(text)

    def replace_code(match: re.Match[str]) -> str:
        code_text = html.escape(match.group(1))
        return (
            '<font name="Courier" backcolor="#F2F2F2">'
            f"{code_text}"
            "</font>"
        )

    return INLINE_CODE_RE.sub(replace_code, escaped)


def build_styles():
    base = getSampleStyleSheet()

    styles = {
        "title": ParagraphStyle(
            "GuideTitle",
            parent=base["Title"],
            fontName="Helvetica-Bold",
            fontSize=22,
            leading=28,
            spaceAfter=12,
            textColor=colors.HexColor("#16324F"),
        ),
        "h1": ParagraphStyle(
            "GuideH1",
            parent=base["Heading1"],
            fontName="Helvetica-Bold",
            fontSize=18,
            leading=22,
            spaceBefore=14,
            spaceAfter=8,
            textColor=colors.HexColor("#16324F"),
        ),
        "h2": ParagraphStyle(
            "GuideH2",
            parent=base["Heading2"],
            fontName="Helvetica-Bold",
            fontSize=14,
            leading=18,
            spaceBefore=12,
            spaceAfter=6,
            textColor=colors.HexColor("#204C6F"),
        ),
        "h3": ParagraphStyle(
            "GuideH3",
            parent=base["Heading3"],
            fontName="Helvetica-Bold",
            fontSize=11.5,
            leading=15,
            spaceBefore=10,
            spaceAfter=5,
            textColor=colors.HexColor("#204C6F"),
        ),
        "body": ParagraphStyle(
            "GuideBody",
            parent=base["BodyText"],
            fontName="Helvetica",
            fontSize=10,
            leading=14,
            spaceAfter=4,
        ),
        "bullet": ParagraphStyle(
            "GuideBullet",
            parent=base["BodyText"],
            fontName="Helvetica",
            fontSize=10,
            leading=14,
            leftIndent=14,
            firstLineIndent=0,
            spaceAfter=3,
        ),
        "number": ParagraphStyle(
            "GuideNumber",
            parent=base["BodyText"],
            fontName="Helvetica",
            fontSize=10,
            leading=14,
            leftIndent=10,
            firstLineIndent=0,
            spaceAfter=3,
        ),
        "code": ParagraphStyle(
            "GuideCode",
            parent=base["Code"],
            fontName="Courier",
            fontSize=8,
            leading=10,
            leftIndent=10,
            rightIndent=6,
            spaceBefore=4,
            spaceAfter=8,
            backColor=colors.HexColor("#F5F7FA"),
            borderColor=colors.HexColor("#D6DCE5"),
            borderWidth=0.5,
            borderPadding=6,
            borderRadius=2,
        ),
    }

    return styles


def add_page_number(canvas, doc):
    canvas.saveState()
    canvas.setFont("Helvetica", 8)
    canvas.setFillColor(colors.HexColor("#6B7280"))
    canvas.drawRightString(doc.pagesize[0] - 18 * mm, 10 * mm, f"Page {doc.page}")
    canvas.restoreState()


def flush_paragraph(buffer: list[str], story: list, style: ParagraphStyle):
    if not buffer:
        return
    text = " ".join(part.strip() for part in buffer if part.strip())
    if text:
        story.append(Paragraph(style_inline_code(text), style))
    buffer.clear()


def flush_code(buffer: list[str], story: list, style: ParagraphStyle):
    if not buffer:
        return
    code = "\n".join(buffer).rstrip()
    if code:
        story.append(XPreformatted(html.escape(code), style))
    buffer.clear()


def markdown_to_story(markdown_text: str, styles: dict[str, ParagraphStyle]):
    story = []
    paragraph_buffer: list[str] = []
    code_buffer: list[str] = []
    in_code_block = False
    first_title_seen = False

    for raw_line in markdown_text.splitlines():
        line = raw_line.rstrip("\n")

        if line.startswith("```"):
            flush_paragraph(paragraph_buffer, story, styles["body"])
            if in_code_block:
                flush_code(code_buffer, story, styles["code"])
                in_code_block = False
            else:
                in_code_block = True
            continue

        if in_code_block:
            code_buffer.append(line)
            continue

        stripped = line.strip()
        if not stripped:
            flush_paragraph(paragraph_buffer, story, styles["body"])
            if story and not isinstance(story[-1], Spacer):
                story.append(Spacer(1, 4))
            continue

        if stripped.startswith("# "):
            flush_paragraph(paragraph_buffer, story, styles["body"])
            text = style_inline_code(stripped[2:].strip())
            story.append(Paragraph(text, styles["title"] if not first_title_seen else styles["h1"]))
            first_title_seen = True
            continue

        if stripped.startswith("## "):
            flush_paragraph(paragraph_buffer, story, styles["body"])
            story.append(Paragraph(style_inline_code(stripped[3:].strip()), styles["h1"]))
            continue

        if stripped.startswith("### "):
            flush_paragraph(paragraph_buffer, story, styles["body"])
            story.append(Paragraph(style_inline_code(stripped[4:].strip()), styles["h2"]))
            continue

        if stripped.startswith("#### "):
            flush_paragraph(paragraph_buffer, story, styles["body"])
            story.append(Paragraph(style_inline_code(stripped[5:].strip()), styles["h3"]))
            continue

        if stripped.startswith("- "):
            flush_paragraph(paragraph_buffer, story, styles["body"])
            story.append(
                Paragraph(
                    style_inline_code(stripped[2:].strip()),
                    styles["bullet"],
                    bulletText="•",
                )
            )
            continue

        numbered_match = NUMBERED_RE.match(stripped)
        if numbered_match:
            flush_paragraph(paragraph_buffer, story, styles["body"])
            story.append(
                Paragraph(
                    style_inline_code(numbered_match.group(2)),
                    styles["number"],
                    bulletText=f"{numbered_match.group(1)}.",
                )
            )
            continue

        paragraph_buffer.append(stripped)

    flush_paragraph(paragraph_buffer, story, styles["body"])
    flush_code(code_buffer, story, styles["code"])
    return story


def build_pdf(markdown_path: Path, pdf_path: Path):
    styles = build_styles()
    markdown_text = markdown_path.read_text(encoding="utf-8")
    story = markdown_to_story(markdown_text, styles)

    pdf_path.parent.mkdir(parents=True, exist_ok=True)
    doc = SimpleDocTemplate(
        str(pdf_path),
        pagesize=A4,
        leftMargin=18 * mm,
        rightMargin=18 * mm,
        topMargin=18 * mm,
        bottomMargin=16 * mm,
        title="CC1101 Guide And DoorGarageFob Deep Dive",
        author="OpenAI Codex",
    )
    doc.build(story, onFirstPage=add_page_number, onLaterPages=add_page_number)


def main():
    markdown_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("docs/CC1101_Guide.md")
    pdf_path = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("docs/CC1101_Guide.pdf")
    build_pdf(markdown_path, pdf_path)
    print(f"Built {pdf_path}")


if __name__ == "__main__":
    main()
