BOX_OUTER_WIDTH = 100;
BOX_OUTER_LENGTH = 65;
BOX_OUTER_HEIGHT = 55;

function outerShell() {
  translate([-BOX_OUTER_WIDTH/2, -BOX_OUTER_LENGTH/2, 0]) {
    cube([
      BOX_OUTER_WIDTH,
      BOX_OUTER_LENGTH,
      BOX_OUTER_HEIGHT]);
  }
}