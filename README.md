# f7disco
template workspace for the stm32f769 discovery board

File structure is arranged as follows:

(home)
	|---stm32f7
	|		|---f7disco 	  <--- This repo
	|		|---STM32_Cube_F7 <--- github/trentgill
	|		|---other_proj
	|		|---...
	|		|---lua           <--- github/trentgill
	|
	|---stm32f4
	|		|---...
	|
	|---wrLib 				  <--- github/whimsicalraps

/STM32Cube  git pull https://github.com/trentgill/STM32_Cube_F7.git
/lua        git pull https://github.com/trentgill/lua.git
/wrLib      git pull https://github.com/whimsicalraps/wrLib.git

# Roadmap

- wrLib integration: osc / gate arrangement
- usb stack needs to work with more keyboards
	+ currently limited to very small subset
- cleanup usb stack & implement dynamic discovery / connection
- lua callbacks to C functions via command line
- design interaction between script writing & use (not just shell)
- simple audio keywords for lua->dsp.params
- variable dsp-graph implementation
- terminal based UI?!



// sys architecture


init(){
	serial_debugger()
	USB_stack()
	LEDs()
	audio_codec()
	lcd_screen()

	print('rtg')
}

main( lowest priority infinite loop ){
	lcd_redraw( if dirty flag set )
}

callbacks(){
	serial_rx(unimplemented)(
		low priority
		needs a buffer in case of high throughput)
	USB_device_detect(
		sporadic
		must be low priority)
	USB_event(
		freq: 1kHz
		calls lua 'EVAL' -> calls C functions
		might take a long time)
	audio_block(
		freq: 48kHz
		high % of cpu use)
	lua_callback(
		internal timers?)
	timer(
		freq: set at runtime
		low priority)
}

/* implementation details */

// audio_block should remain in irq (prioity 0)
	// needs to drop frames if out of time (not stack irqs)

// serial_rx:
	// push data to buffer & set flag for main loop

// usb device change:
	// unclear what needs to happen here

// usb event:
	// ??

// timer:
	// ??
