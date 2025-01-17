#include <string.h>
#include "JWModules.hpp"

struct ThingThingBall {
	NVGcolor color;
};

struct ThingThing : Module {
	enum ParamIds {
		BALL_RAD_PARAM,
		ZOOM_MULT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		BALL_RAD_INPUT,
		ZOOM_MULT_INPUT,
		ANG_INPUT,
		NUM_INPUTS = ANG_INPUT + 5
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	ThingThingBall *balls = new ThingThingBall[5];
	float atten[5] = {1, 1, 1, 1, 1};
	// float atten[5] = {0.0, 0.25, 0.5, 0.75, 1};

	ThingThing() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BALL_RAD_PARAM, 0.0, 30.0, 10.0);
		configParam(ZOOM_MULT_PARAM, 1.0, 200.0, 20.0);
		balls[0].color = nvgRGB(255, 255, 255);//white
		balls[1].color = nvgRGB(255, 151, 9);//orange
		balls[2].color = nvgRGB(255, 243, 9);//yellow
		balls[3].color = nvgRGB(144, 26, 252);//purple
		balls[4].color = nvgRGB(25, 150, 252);//blue
	}
	~ThingThing() {
		delete [] balls;
	}

	void process(const ProcessArgs &args) override {};
	void onReset() override {}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {}
};

struct ThingThingDisplay : Widget {
	ThingThing *module;
	ThingThingDisplay(){}

	void draw(const DrawArgs &args) override {
		//background
		nvgFillColor(args.vg, nvgRGB(20, 30, 33));
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFill(args.vg);

		if(module == NULL) return;

		float ballRadius = module->params[ThingThing::BALL_RAD_PARAM].getValue();
		if(module->inputs[ThingThing::BALL_RAD_INPUT].isConnected()){
			ballRadius += rescalefjw(module->inputs[ThingThing::BALL_RAD_INPUT].getVoltage(), -5.0, 5.0, 0.0, 30.0);
		}

		float zoom = module->params[ThingThing::ZOOM_MULT_PARAM].getValue();
		if(module->inputs[ThingThing::ZOOM_MULT_INPUT].isConnected()){
			zoom += rescalefjw(module->inputs[ThingThing::ZOOM_MULT_INPUT].getVoltage(), -5.0, 5.0, 1.0, 50.0);
		}

      float x[5];
      float y[5];
      float angle[5];

      for(int i=0; i<5; i++){
         angle[i] = i==0 ? 0 : (module->inputs[ThingThing::ANG_INPUT+i].getVoltage() + angle[i-1]) * module->atten[i];
			x[i] = i==0 ? 0 : sinf(rescalefjw(angle[i], -5, 5, -2*M_PI + M_PI/2.0f, 2*M_PI + M_PI/2.0f)) * zoom;
			y[i] = i==0 ? 0 : cosf(rescalefjw(angle[i], -5, 5, -2*M_PI + M_PI/2.0f, 2*M_PI + M_PI/2.0f)) * zoom;
      }

		/////////////////////// LINES ///////////////////////
		nvgSave(args.vg);
		nvgTranslate(args.vg, box.size.x * 0.5, box.size.y * 0.5);
		for(int i=0; i<5; i++){
			nvgTranslate(args.vg, x[i], y[i]);
			nvgStrokeColor(args.vg, nvgRGB(255, 255, 255));
			if(i>0){
				nvgStrokeWidth(args.vg, 1);
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, 0, 0);
				nvgLineTo(args.vg, -x[i], -y[i]);
				nvgStroke(args.vg);
			}
		}
		nvgRestore(args.vg);

		/////////////////////// BALLS ///////////////////////
		nvgSave(args.vg);
		nvgTranslate(args.vg, box.size.x * 0.5, box.size.y * 0.5);
		for(int i=0; i<5; i++){
			nvgTranslate(args.vg, x[i], y[i]);
			nvgStrokeColor(args.vg, module->balls[i].color);
			nvgFillColor(args.vg, module->balls[i].color);
			nvgStrokeWidth(args.vg, 2);
			nvgBeginPath(args.vg);
			nvgCircle(args.vg, 0, 0, ballRadius);
			nvgFill(args.vg);
			nvgStroke(args.vg);
		}
		nvgRestore(args.vg);
	}
};


struct ThingThingWidget : ModuleWidget {
	ThingThingWidget(ThingThing *module);
};

ThingThingWidget::ThingThingWidget(ThingThing *module) {
	setModule(module);
	box.size = Vec(RACK_GRID_WIDTH*20, RACK_GRID_HEIGHT);

	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ThingThing.svg")));
	addChild(panel);

	ThingThingDisplay *display = new ThingThingDisplay();
	display->module = module;
	display->box.pos = Vec(0, 0);
	display->box.size = Vec(box.size.x, RACK_GRID_HEIGHT);
	addChild(display);

	addChild(createWidget<Screw_J>(Vec(265, 365)));
	addChild(createWidget<Screw_W>(Vec(280, 365)));

	for(int i=0; i<4; i++){
		addInput(createInput<TinyPJ301MPort>(Vec(5+(20*i), 360), module, ThingThing::ANG_INPUT+i+1));
	}
	
	addInput(createInput<TinyPJ301MPort>(Vec(140, 360), module, ThingThing::BALL_RAD_INPUT));
	addParam(createParam<JwTinyKnob>(Vec(155, 360), module, ThingThing::BALL_RAD_PARAM));

	addInput(createInput<TinyPJ301MPort>(Vec(190, 360), module, ThingThing::ZOOM_MULT_INPUT));
	addParam(createParam<JwTinyKnob>(Vec(205, 360), module, ThingThing::ZOOM_MULT_PARAM));
}

Model *modelThingThing = createModel<ThingThing, ThingThingWidget>("ThingThing");
