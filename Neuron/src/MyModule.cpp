#include "Template.hpp"

#include "OOPS.h"

#define MAX_FREQUENCY_TIMESTEP 0.00005f
#define MAX_VOLTAGE 20.0f
// Default timestep based on 262Hz
#define DEFAULT_FREQUENCY_TIMESTEP 0.00381679389f
#define DEFAULT_VOLTAGE 5.0f
#define VOLT_PER_OCTAVE_TIMESTEP DEFAULT_FREQUENCY_TIMESTEP / DEFAULT_VOLTAGE

// Alright so we need time step to map such that 1V/O => 1 volt increase == 0.5*timestep increase
float voltageToTimestepVoltPerOctave(float voltage) {
    return VOLT_PER_OCTAVE_TIMESTEP * voltage * 0.25f;
}


struct NeuronModule : Module {

	enum Params
	{
		P1=0,P2,P3,P4,P5,P6,P7,P8
	};

	enum Type
	{
		BASE=0,GAIN,IN
	};
    enum InputNames {
        PITCH=0,
        CURRENT,
        LITHIUM,
        SODIUM,
        POTASSIUM,
        V1,
        V2,
        v3
    };

	enum ParamIds {
		P1_BASE,P1_GAIN,
		P2_BASE,P2_GAIN,
		P3_BASE,P3_GAIN,
		P4_BASE,P4_GAIN,
		P5_BASE,P5_GAIN,
		P6_BASE,P6_GAIN,
		P7_BASE,P7_GAIN,
		P8_BASE,P8_GAIN,
		NUM_PARAMS
	};
	enum InputIds {
		P1_IN,
		P2_IN,
		P3_IN,
		P4_IN,
		P5_IN,
		P6_IN,
		P7_IN,
		P8_IN,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	float phase = 0.0;
	float blinkPhase = 0.0;

	tNeuron* neuron;
	tRamp* env;
	tCompressor* compressor;

	float inputVoltages[8][3];
    float sumVoltages[8];

	NeuronModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		OOPSInit(engineGetSampleRate(), nullptr);

		neuron = tNeuronInit();
		neuron->current = 100.0f;

		env = tRampInit(10, 1);

		compressor = tCompressorInit();
    	compressor->M = 1.0f;
    	compressor->T = 0.0f;
    	compressor->tauAttack = 25.0f;
    	compressor->tauRelease = 250.0f;
    	compressor->R = 12.0f;
    	compressor->W = 6.0f;
	}
	void step() override;


	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

int counter;

void NeuronModule::step() {
	// Implement a simple sine oscillator
	float deltaTime = 1.0 / engineGetSampleRate();

	// Compute the frequency from the pitch parameter and input
	//float pitch = params[PITCH_PARAM].value;
	//pitch += inputs[PITCH_INPUT].value;
	//pitch = clampf(pitch, -4.0, 4.0);
	//float freq = 440.0 * powf(2.0, pitch);

	for (int i = 0 ; i < 8; i++)
	{
		inputVoltages[i][BASE] = 0.5f * MAX_VOLTAGE * params[i*2].value;
		inputVoltages[i][GAIN] = params[i*2+1].value;
		inputVoltages[i][IN] = (inputs[i].value);
		sumVoltages[i] =  (inputVoltages[i][BASE] + inputVoltages[i][GAIN] * inputVoltages[i][IN]);
	}

    float current = 90.0f+10.0f*(sumVoltages[CURRENT]);
    float timeStep = voltageToTimestepVoltPerOctave(sumVoltages[PITCH]);
    float lithium = -sumVoltages[LITHIUM];
    float sodium = 128.0f + (sumVoltages[SODIUM] * 128.0f) * 3.0f;
    float potassium = 70.0f + sumVoltages[POTASSIUM] * 50.0f;
    float v1 = (sumVoltages[V1] * 128.0f)*2.0f - 128.0f;
    float v3 = (sumVoltages[V2] * 128.0f)*2.0f - 128.0f;

    if (++counter == 80000)
    {
        counter = 0;
        printf("timestep: %f \n", timeStep);
        printf("base   : %f\n", inputVoltages[PITCH][BASE]);
        // for (int i = 0 ; i < 3; i++)
        // {
        //     printf("P%d base   : %f\n", i + 1, inputVoltages[i][BASE]);
        //     printf("P%d gain   : %f\n", i + 1, inputVoltages[i][GAIN]);
        //     printf("P%d in     : %f\n", i + 1, inputVoltages[i][IN]);
        //     printf("P%d sumVoltage : %f\n", i + 1, sumVoltages[i]);
        // }
    }



    // Neuron expects a negative timestep???????
    tNeuronSetTimeStep(neuron, -timeStep);
    tNeuronSetCurrent(neuron, current);
	tNeuronSetL(neuron, lithium);
	tNeuronSetN(neuron, sodium);
    tNeuronSetK(neuron, potassium);
    tNeuronSetV1(neuron, v1);
    tNeuronSetV3(neuron, v3);

    //tNeuronSetL(neuron, -(crunched[P3] * 128.0f));
    /*
    tNeuronSetN(neuron, 128.0f + cval * 3.0f);

    tNeuronSetC(neuron, in * 2.0f + 0.01);
    tNeuronSetV1(neuron, cval*2.0f - 128.0f);
    tNeuronSetV3(neuron, cval*2.0f - 128.0f);
    tNeuronSetTimeStep(neuron, 1.0f / (cval * 2.0f + 1.0f));
    */


	outputs[OUTPUT].value = 10.0f * tCompressorTick(compressor, tNeuronTick(neuron));

	// Blink light at 1Hz
	blinkPhase += deltaTime;
	if (blinkPhase >= 1.0)
		blinkPhase -= 1.0;
	lights[BLINK_LIGHT].value = (blinkPhase < 0.5) ? 1.0 : 0.0;
}

#define USE_BACKGROUND 1

NeuronModuleWidget::NeuronModuleWidget() {
	NeuronModule *module = new NeuronModule();
	setModule(module);
	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
#if USE_BACKGROUND
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/neuron.svg")));
		addChild(panel);
	}
#endif

	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	float y_spacing = RACK_GRID_HEIGHT / 5;

	// COLUMN 1
	int offset = -5;
	int davies_x = 10;
	int bef_x = davies_x + 45;
	int pj_x = davies_x + 23;

	int y = offset + y_spacing;
	addParam(createParam<Davies1900hBlackKnob>(Vec(davies_x, y), module, 	NeuronModule::P1_BASE, 0.0, 1.0, 0.5));
	addParam(createParam<BefacoTinyKnob>(Vec(bef_x, y+5), module, 			NeuronModule::P1_GAIN, 0.0, 1.0, 0.5));
	addInput(createInput<PJ301MPort>(Vec(pj_x, y + 40), module, 			NeuronModule::P1_IN));

	y += y_spacing;
	addParam(createParam<Davies1900hBlackKnob>(Vec(davies_x, y), module, 	NeuronModule::P2_BASE, 0.0, 1.0, 0.5));
	addParam(createParam<BefacoTinyKnob>(Vec(bef_x, y+5), module, 			NeuronModule::P2_GAIN, 0.0, 1.0, 0.5));
	addInput(createInput<PJ301MPort>(Vec(pj_x, y + 40), module, 			NeuronModule::P2_IN));

	y += y_spacing;
	addParam(createParam<Davies1900hBlackKnob>(Vec(davies_x, y), module, 	NeuronModule::P3_BASE, 0.0, 1.0, 0.5));
	addParam(createParam<BefacoTinyKnob>(Vec(bef_x, y+5), module, 		 	NeuronModule::P3_GAIN, 0.0, 1.0, 0.5));
	addInput(createInput<PJ301MPort>(Vec(pj_x, y + 40), module, 			NeuronModule::P3_IN));

	y += y_spacing;
	addParam(createParam<Davies1900hBlackKnob>(Vec(davies_x, y), module, 	NeuronModule::P4_BASE, 0.0, 1.0, 0.5));
	addParam(createParam<BefacoTinyKnob>(Vec(bef_x, y+5), module, 			NeuronModule::P4_GAIN, 0.0, 1.0, 0.5));
	addInput(createInput<PJ301MPort>(Vec(pj_x, y + 40), module, 			NeuronModule::P4_IN));

	// COLUMN 2

	davies_x += box.size.x * 0.5;
	bef_x += box.size.x * 0.5;
	pj_x += box.size.x * 0.5;

	y = offset + y_spacing;
	addParam(createParam<Davies1900hBlackKnob>(Vec(davies_x, y), module, 	NeuronModule::P5_BASE, 0.0, 1.0, 0.5));
	addParam(createParam<BefacoTinyKnob>(Vec(bef_x, y+5), module, 			NeuronModule::P5_GAIN, 0.0, 1.0, 0.5));
	addInput(createInput<PJ301MPort>(Vec(pj_x, y + 40), module, 			NeuronModule::P5_IN));

	y += y_spacing;
	addParam(createParam<Davies1900hBlackKnob>(Vec(davies_x, y), module, 	NeuronModule::P6_BASE, 0.0, 1.0, 0.5));
	addParam(createParam<BefacoTinyKnob>(Vec(bef_x, y+5), module, 			NeuronModule::P6_GAIN, 0.0, 1.0, 0.5));
	addInput(createInput<PJ301MPort>(Vec(pj_x, y + 40), module, 			NeuronModule::P6_IN));

	y += y_spacing;
	addParam(createParam<Davies1900hBlackKnob>(Vec(davies_x, y), module, 	NeuronModule::P7_BASE, 0.0, 1.0, 0.5));
	addParam(createParam<BefacoTinyKnob>(Vec(bef_x, y+5), module, 			NeuronModule::P7_GAIN, 0.0, 1.0, 0.5));
	addInput(createInput<PJ301MPort>(Vec(pj_x, y + 40), module, 			NeuronModule::P7_IN));

	y += y_spacing;
	addParam(createParam<Davies1900hBlackKnob>(Vec(davies_x, y), module, 	NeuronModule::P8_BASE, 0.0, 1.0, 0.5));
	addParam(createParam<BefacoTinyKnob>(Vec(bef_x, y+5), module, 			NeuronModule::P8_GAIN, 0.0, 1.0, 0.5));
	addInput(createInput<PJ301MPort>(Vec(pj_x, y + 40), module, 			NeuronModule::P8_IN));



	// OUTPUT
	addOutput(createOutput<PJ301MPort>(Vec(box.size.x * 0.5,  y_spacing * 0.5), module, NeuronModule::OUTPUT));
}
