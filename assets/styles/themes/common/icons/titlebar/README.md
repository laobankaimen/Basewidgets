# Title-bar icons

Original BaseWidgets SVG artwork; no icon font or third-party bitmap dependency.
24 × 24 viewBox, common 1.6–1.7 stroke, theme placeholder #03b8e5.
Runtime logical sizes: actions/bell/avatar slot 20 px, app logo 20 px.
Bell and settings share a 24 × 24 canvas and 1.6-unit stroke; do not add
an extra scale transform inside the settings SVG. The avatar circle occupies
90% of its slot (18 px at the default size) to balance the outline icons.
Icon-only action targets remain 36 px wide; glyph size is independent of the hit target.
SVG is rendered at the current paint-device resolution; do not multiply these logical sizes by DPR before QIcon::paint.

Integration and DPI rules: [theme and resources guide](../../../../../../docs/guides/themes-and-resources.md).
