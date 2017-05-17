dspgraph

linked list of all dsp blocks
- store info about signal types (blocks self-regulate internal audio/ctrl choices)
- registration-style connections. each block knows where to grab its input
- master section defines 'layers' of computation
	- each layer is processed sequentially into generic i/o blocks
	- calculated by working backward from dac
	- ^ aides in determining optimizations / blocks that dont need to run
- flags & sections for per-voice vs master blocks
- insert/rm-block function causes rebuild of graph
	- both fns should take care of linking/closing-loop

-- process
look at sources of DAC, then work back up chain
gather / process static settings / inputs
	> make note of any settings that imply optimizations are maybe possible
construct a list of blocks at a heirarchy point along with their sources
continue until list exhausted
	> can be done incrementally when insert/rm/move blocks
apply any optimization flags (dont change graph)
	> this is through blocks functions which set flags in the block
	> block is then responsible for configuring appropriate signal process
	> should implement by modifying a list of function pointers
		- this provides a unified interface to the graph, but allows
		  the block to reconfigure its process to optimize cpu
each block passes a pointer to a dedicated (flipflop?) buffer for that chain
	> max parallel chains is defined by RAM usage
	> could process each channel sequentially to reduce RAM but increase CPU
	> flipflop optional depending if module needs input buffer access
		- module has a 'needs flip' flag and graph takes care of pointers
		- modules dont know about origins/destinations!
		>> each mod just takes a read pointer and sets @ write pointer
	> a module that is optimized out (ie waveshaper) just returns *in (no flip)
note optimizations on global/local boundaries
	> eg calculating 'RAMP' for multiple oscillators
		- ramp calc gets its own heirarchy level above the func-gens
		- calculates block and pushes to temp pointer buf in graph.

as optimization doesnt affect overall graph, it can be recalcd each block
hence even blocks being ctrl-rate modulated can sometimes optimize down

-- roadmap
- tone generator & DAC to start
- add volume (static vol -6db)
- allow chaining multiple volumes
- mixer (2>1). add an oscillator in parallel
	> could be separate block, or generic fn in block, or implicit in grapher
