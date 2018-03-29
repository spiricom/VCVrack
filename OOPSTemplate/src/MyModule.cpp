#include "Template.hpp"

#include "OOPS.h"


struct OscillatorModule : Module {
	enum ParamIds {
		PITCH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SINE_OUTPUT,
		SAW_OUTPUT,
		TRI_OUTPUT,
		SQUARE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	float phase = 0.0;
	float blinkPhase = 0.0;

	tSawtooth* saw;
	tCycle* sine;
	tTriangle* tri;
	tSquare* square;

	OscillatorModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) 
	{
		OOPSInit(engineGetSampleRate(), nullptr);

		saw = tSawtoothInit();
		sine = tCycleInit();
		tri = tTriangleInit();
		square = tSquareInit();
	}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void OscillatorModule::step() {
	// Implement a simple sine oscillator
	float deltaTime = 1.0 / engineGetSampleRate();

	// Compute the frequency from the pitch parameter and input
	float pitch = params[PITCH_PARAM].value;
	pitch += inputs[PITCH_INPUT].value;
	pitch = clampf(pitch, -4.0, 4.0);
	float freq = 440.0 * powf(2.0, pitch);

	//tSawtoothSetFreq(&osc, freq);
	//outputs[SINE_OUTPUT].value = tSawtoothTick(&osc);

	tSawtoothSetFreq(saw, freq);
	tTriangleSetFreq(tri, freq);
	tCycleSetFreq(sine, freq);
	tSquareSetFreq(square, freq);

	outputs[SINE_OUTPUT].value = tCycleTick(sine);
	outputs[SAW_OUTPUT].value = tSawtoothTick(saw);
	outputs[TRI_OUTPUT].value = tTriangleTick(tri);
	outputs[SQUARE_OUTPUT].value = tSquareTick(square);

	

	// Blink light at 1Hz
	blinkPhase += deltaTime;
	if (blinkPhase >= 1.0)
		blinkPhase -= 1.0;
	lights[BLINK_LIGHT].value = (blinkPhase < 0.5) ? 1.0 : 0.0;
}

#define USE_BACKGROUND 1

OscillatorModuleWidget::OscillatorModuleWidget() {
	OscillatorModule *module = new OscillatorModule();
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
#if USE_BACKGROUND
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/OhNoes.svg")));
		addChild(panel);
	}
#endif

	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	float y_spacing = RACK_GRID_HEIGHT / 7;
	int which = 1;
	addParam(createParam<Davies1900hBlackKnob>(Vec(28, y_spacing * which++), module, OscillatorModule::PITCH_PARAM, -3.0, 3.0, 0.0));

	addInput(createInput<PJ301MPort>(Vec(33, y_spacing * which++), module, OscillatorModule::PITCH_INPUT));

	addOutput(createOutput<PJ301MPort>(Vec(33, y_spacing * which++), module, OscillatorModule::SINE_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(33, y_spacing * which++), module, OscillatorModule::TRI_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(33, y_spacing * which++), module, OscillatorModule::SAW_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(33, y_spacing * which++), module, OscillatorModule::SQUARE_OUTPUT));

	addChild(createLight<MediumLight<RedLight>>(Vec(41, 40), module, OscillatorModule::BLINK_LIGHT));
}

