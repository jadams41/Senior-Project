extern void halt_wrapper();
extern void VGA_clear();
extern void VGA_display_char(char);
extern void VGA_display_str(const char *);

int kmain(){
  int enabled = 0;

  //infinite loop to allow us to attach gdb before the program finishes executing
  //while(!enabled) {
  //;//halt_wrapper();
  //}

  //if we make it here, we know that gdb was attached and used to break out of infinite loop

  //clear the vga console now that we are up and running
  VGA_clear();

  while(!enabled) ;

  //try writing a couple of characters to the screen
  int i;
  for(i = 0; i < 80; i++){
	VGA_display_char('t');
  }
  VGA_display_char('\n');
  VGA_display_char('x');
  VGA_display_char('x');
  VGA_display_char('x');
  VGA_display_char('\n');
  VGA_display_char('b');
  // VGA_display_char('s');
  // VGA_display_char('t');


}
