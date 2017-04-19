# f7disco
template workspace for the stm32f769 discovery board

File structure is arranged as follows:

(home)
	|---stm32f7
	|		|---f7disco 	<--- This repo
	|		|---STM32Cube	<--- github/trentgill
	|		|---other_proj
	|		|---...
	|
	|---stm32f4
	|		|---...
	|
	|---wrLib 				<--- github/whimsicalraps

/STM32Cube 	git pull https://github.com/trentgill/STM32_Cube_F7.git
/wrLib 		git pull https://github.com/whimsicalraps/wrLib.git

# Roadmap

- usart to accept strings from minicom
- wrLib integration: osc / gate arrangement
- lua vm running on board
- lua interpreter running
- add a general purpose timer that sets flag in main loop
- simple audio keywords for lua->dsp.params
- usb-hid keyboard support direct to the shell