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

- usart to accept strings from minicom
	- once doing string entry, need to move screen refresh to timer
		> otherwise it will be called from uart interrupt!
- wrLib integration: osc / gate arrangement
- lua vm running on board
- lua interpreter running
- add a general purpose timer that sets flag in main loop
- simple audio keywords for lua->dsp.params
- usb-hid keyboard support direct to the shell