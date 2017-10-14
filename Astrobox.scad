BOX_OUTER_WIDTH = 106;
BOX_OUTER_LENGTH = 81;
BOX_OUTER_HEIGHT = 55;
THICKNESS = 3;
BUTTON_HOLE_D = 18;
SWITCH_WIDTH = 14.3;
SWITCH_LENGTH = 24.3;
SWITCH_SCREW_INSET = 2;
SWITCH_SCREW_OFFSET = SWITCH_WIDTH/2;
SWITCH_HOLE_D = 4;
TOGGLE_HOLE_D = 13.3;
POT_HOLE_D = 7.3;
POT_KNOB_D = 24;

LCD_DISPLAY_WIDTH = 71.5;
LCD_DISPLAY_HEIGHT = 26.5;
LCD_ANCHOR_WIDTH = 80;
LCD_ANCHOR_HEIGHT = 36.3;
LCD_HOLE_INSET = 1.5;
LCD_HOLE_D = 3;

$fn = 100;

 module prism(l, w, h){
   polyhedron(
       points=[[0,0,0], [l,0,0], [l,w,0], [0,w,0], [0,w,h], [l,w,h]],
       faces=[[0,1,2,3],[5,4,3,2],[0,4,5,1],[0,3,4],[5,2,1]]
   );
};
       
module cantilever(s,r) {
    CANTILEVER_LENGTH=10*s;
    CANTILEVER_HEIGHT=12*s;
    CANTILEVER_WIDTH=2*s;
    CANTILEVER_DEPTH=3*s;
    CANTILEVER_TAB=2*s;
    CANTILEVER_BASE_HEIGHT=5*s;
    CANTILEVER_BASE_FACTOR=2;
    cantilever_base = CANTILEVER_BASE_FACTOR * CANTILEVER_HEIGHT;
    rotate(r) {
      translate([-(CANTILEVER_WIDTH/2),-(CANTILEVER_HEIGHT/2), 0]){
          cube([CANTILEVER_WIDTH,CANTILEVER_HEIGHT,CANTILEVER_LENGTH + CANTILEVER_BASE_HEIGHT]);
          translate([CANTILEVER_WIDTH,CANTILEVER_HEIGHT,CANTILEVER_LENGTH+CANTILEVER_BASE_HEIGHT]){
              rotate([90,180,90]) {
                prism(CANTILEVER_HEIGHT, CANTILEVER_DEPTH, CANTILEVER_TAB);
              };
          };
      };
      FLARE_DIAMETER=cantilever_base - CANTILEVER_WIDTH;
      translate([-(cantilever_base/2), -(CANTILEVER_HEIGHT/2), 0]) {
          difference() {
              cube([cantilever_base, CANTILEVER_HEIGHT, CANTILEVER_BASE_HEIGHT]);
              union() {
                  translate([0,CANTILEVER_HEIGHT+1,CANTILEVER_BASE_HEIGHT]){
                      rotate([90,0,0]){
                          cylinder(h=CANTILEVER_HEIGHT+2, d=FLARE_DIAMETER, center=false);
                      };
                  };
                  translate([cantilever_base,CANTILEVER_HEIGHT+1,CANTILEVER_BASE_HEIGHT]){
                      rotate([90,0,0]){
                          cylinder(h=CANTILEVER_HEIGHT+2, d=FLARE_DIAMETER, center=false);
                      };
                  };
              };
          };
     };
  };
};

module cantilever_tab(s,r){
    CANTILEVER_LENGTH=10*s;
    CANTILEVER_HEIGHT=12*s;
    CANTILEVER_WIDTH=2*s;
    CANTILEVER_DEPTH=3*s;
    CANTILEVER_TAB=2*s;
    CANTILEVER_BASE_HEIGHT=5*s;
    CANTILEVER_BASE_FACTOR=2;
    cantilever_base = CANTILEVER_BASE_FACTOR * CANTILEVER_HEIGHT;
    rotate(r) {
      translate([-(CANTILEVER_WIDTH/2), -(CANTILEVER_HEIGHT/2)-0.05, -0.1]) {
          difference() {
            cube([2*CANTILEVER_WIDTH,CANTILEVER_HEIGHT+0.1,CANTILEVER_LENGTH + CANTILEVER_BASE_HEIGHT]);
            translate([(2*CANTILEVER_WIDTH)-(CANTILEVER_WIDTH*0.69),-0.1,CANTILEVER_HEIGHT-CANTILEVER_DEPTH+0.1]) {
              cube([CANTILEVER_WIDTH*0.7, CANTILEVER_HEIGHT+0.21, CANTILEVER_DEPTH]);
          };
        };
      };
    };
};

module Box(thickness){
  translate([-BOX_OUTER_WIDTH/2, -BOX_OUTER_LENGTH/2, 0]) {
    difference() {
      cube([
        BOX_OUTER_WIDTH,
        BOX_OUTER_LENGTH,
        BOX_OUTER_HEIGHT]);
      translate([thickness,thickness,thickness])
        cube([
          BOX_OUTER_WIDTH-(thickness*2),
          BOX_OUTER_LENGTH-(thickness*2),
          BOX_OUTER_HEIGHT-(thickness)+1]);
    }
  }
};

module Lid(thickness){
  translate([-BOX_OUTER_WIDTH/2, - BOX_OUTER_LENGTH/2, BOX_OUTER_HEIGHT]) {
      cube([
        BOX_OUTER_WIDTH,
        BOX_OUTER_LENGTH,
        thickness]);
  }
};

CANTILEVER_S = 0.5;
translate([0,0,BOX_OUTER_HEIGHT]) {
  translate([-BOX_OUTER_WIDTH/2+THICKNESS,-BOX_OUTER_LENGTH/2+(3*THICKNESS)]) {
    cantilever(0.5,[180,0,180]);
  }
  translate([BOX_OUTER_WIDTH/2-THICKNESS,-BOX_OUTER_LENGTH/2+(3*THICKNESS)]) {
    cantilever(0.5,[180,0,0]);
  }
  translate([BOX_OUTER_WIDTH/2-THICKNESS,BOX_OUTER_LENGTH/2-(3*THICKNESS)]) {
    cantilever(0.5,[180,0,0]);
  }
  translate([-BOX_OUTER_WIDTH/2+THICKNESS,BOX_OUTER_LENGTH/2-(3*THICKNESS)]) {
    cantilever(0.5,[180,0,180]);
  }
}
Lid(THICKNESS);

/*difference() {
  Box(THICKNESS);
  union() {
    translate([0,0,BOX_OUTER_HEIGHT]) {
      translate([-(BOX_OUTER_WIDTH/2)+THICKNESS,-(BOX_OUTER_LENGTH/2)+(3*THICKNESS),0]) {
        cantilever_tab(0.5,[180,0,180]);
      }
      translate([BOX_OUTER_WIDTH/2-THICKNESS,-BOX_OUTER_LENGTH/2+(3*THICKNESS)]) {
        cantilever_tab(0.5,[180,0,0]);
      }
      translate([BOX_OUTER_WIDTH/2-THICKNESS,BOX_OUTER_LENGTH/2-(3*THICKNESS)]) {
        cantilever_tab(0.5,[180,0,0]);
      }
      translate([-BOX_OUTER_WIDTH/2+THICKNESS,BOX_OUTER_LENGTH/2-(3*THICKNESS)]) {
        cantilever_tab(0.5,[180,0,180]);
      }
      translate([-BOX_OUTER_WIDTH/4-5,-BOX_OUTER_LENGTH/2-2,-9.9
      ]) {
        cube([10,10,10]);
      }
    }
    // Front panel
    translate([-34,-24,-0.1]) {
      cylinder(h=THICKNESS+0.2, d=BUTTON_HOLE_D,center=false);
      translate([22,0,0]) {
        cylinder(h=THICKNESS+0.2, d=BUTTON_HOLE_D,center=false);
        translate([22,0,0]) {
          cylinder(h=THICKNESS+0.2, d=BUTTON_HOLE_D, center=false);
        }
      }
      translate([0,22,0]){
        cylinder(h=THICKNESS+0.2, d=BUTTON_HOLE_D,center=false);
        translate([22,0,0]) {
          cylinder(h=THICKNESS+0.2, d=BUTTON_HOLE_D,center=false);
          translate([22,0,0]) {
            cylinder(h=THICKNESS+0.2, d=BUTTON_HOLE_D, center=false);
          }
        }
      }
    }
    translate([35, -19, -0.1]) {
      cylinder(h=THICKNESS+0.2, d=POT_HOLE_D, center=false);
    }
    translate([-34, 16, -0.1]) {
      cube([SWITCH_LENGTH, SWITCH_WIDTH, THICKNESS+0.2]);
      translate([-SWITCH_SCREW_OFFSET/2, SWITCH_SCREW_OFFSET, 0]) {
        cylinder(h=THICKNESS+0.2, d=SWITCH_HOLE_D, center=false);
      
        translate([SWITCH_LENGTH+SWITCH_SCREW_OFFSET, 0, 0]) {
          cylinder(h=THICKNESS+0.2, d=SWITCH_HOLE_D, center=false);
        }
      }
    }
    translate([36, 20, -0.1]) {
      cylinder(h=THICKNESS+0.2, d=TOGGLE_HOLE_D, center=false);
    }
  }
}*/