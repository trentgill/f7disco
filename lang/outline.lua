-- lua frontend to embedded C synth box

synthesis graph control (patching)
	oscillator config (at least 2 ops)
	audio processing path
	audio rate modulations
	this is the DSP section (as simple as possible)

synthesis param control (knobs / sliders)
	setting static values
	connecting mod sources
	slew rates / vactrols
	handle navigation like a multi-tasking window manager
		- hold alt to display graph as overlay w indication of selection
		- each node has a num for RAM or press tab to cycle
		- '[/]' move left/right and ~ cycles vertically

note control / polyphony
	subset of param control
	essentially just setting pitch parameter of 




dsp blocks
	vari-cpu (versions for ctrl rate / audio rate mod)
		> requires an explicit switch (which dsp path?)
		> switching ctrled by graph-handler
	method to freely transform structure
	method to select traditional structures
		> karplus strong
		> 2op FM
		> 2op subtractive
		> 2op formant + AM
	units
		> noise/chaos
		> sisters
			> dynamic dsp usage based on patching
		> delay w/ fb+ff
			> sub-frame delay only w internal fb/ff
			> else fb loop processing as special graph entry
				> rolls processor into delay loop
				> each block reports its latency dynamically
					:can be done practically after the fact
		> sine oscillator (JF) for AM or as source
		> func gen (JF) (each mode a diff colour)
			> audio source: ramp->tri->saw waves
			> audio mod: formant waveshaper / pitch div
			> ctrl source: lfo or envelope
			> graph-handler is smart and looks for
				fg>fg.trigger: only calculates tz trigger
				mod>fg.ramp: audio/event rate based on input
				fg>ws where ws is fixed:
					if sine & ramp=0: calc sine directly
					if sq: calc PWM directly
		> waveshaper (JF)
			> linear transform w LUT
		> math (any interconnection can have one)
			> basic block for volume, but with optionals:
			> lpg (both) behaviour
			> xfade / pan
			> peak / trough
			> rectifier
			> integrator / differentiator
	every unit calculates cpu utilization and system
		reports max polyphony in corner of screen	

.---.---.   .---.     .---.     .---.
|   |   |   |   |   * |   |   * |   |
|   |   |-.-|   |--0--|   |--0--|   |
|   |   |   |   |  |  |   |     |   |
.---.---.   .---.  |  .---.     .---.
                 .---.
                 |   |
                 |   |
                 |   |
                 .---.
1 blk   add blk     select dot      make a math     add input
.---.   .---.---.   .---.   .---.   .---.   .---.   .---.   .---.
|   | > |   |   | > |   |   |   | > |   |  *|   | > |   |  *|   |
|   | > |   |   | > |   |-.-|   | > |   |-0-|   | > |   |-0-|   |
|   | > |   |   | > |   |   |   | > |   |   |   | > |   | | |   |
.---.   .---.---.   .---.   .---.   .---.   .---.   .---. | .---.
                                                        .---.
                                                        |   |
                                                        |   |
                                                        |   |
                                                        .---.
-- above you select the '.' and activate to create 0(math)
-- the '*' represents the math function being used
-- some '*'s have multiple inputs
-- some modules auto direct join (no dot) but can be added


K-S
	noise
	math(vol) < trigger
	delay <~filter < envelope over FB & pitch over time
	filter >^
	>

formant
	func-gen(o)
	func-gen(w) < func-gen 