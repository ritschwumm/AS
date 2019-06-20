//**************************************************************************************
//
//BPM Calculator module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//### BPM detect portions of code by Tomasz Sosnowski - KoralFX
//**************************************************************************************

#include "AS.hpp"

//#include "dsp/digital.hpp"

#include <sstream>
#include <iomanip>


struct BPMCalc2 : Module {
	enum ParamIds {    
    TEMPO_PARAM,      
		NUM_PARAMS
	};  
	enum InputIds {
    CLOCK_INPUT,  
		NUM_INPUTS
	};
	enum OutputIds {  
    ENUMS(MS_OUTPUT, 16),   
		NUM_OUTPUTS
	};
  enum LightIds {
    CLOCK_LOCK_LIGHT,
    CLOCK_LIGHT,
		NUM_LIGHTS
	};

  //bpm detector variables
  bool inMemory = false;
  bool beatLock = false;
  float beatTime = 0.0f;
  int beatCount = 0;
  int beatCountMemory = 0;
  float beatOld = 0.0f;

  std::string tempo = "---";

  dsp::SchmittTrigger clockTrigger;
  dsp::PulseGenerator LightPulse;
  bool pulse = false;

  //calculator variables
  float bpm = 120;
  float last_bpm = 0;
  float millisecs = 60000;
  float mult = 1000;
  float millisecondsPerBeat;
  float millisecondsPerMeasure;
  float bar = 1.0f;

  float secondsPerBeat = 0.0f;
	float secondsPerMeasure = 0.0f;
  //ms variables
  float half_note_d = 1.0f;
  float half_note = 1.0f;
  float half_note_t =1.0f;

  float qt_note_d = 1.0f;
  float qt_note = 1.0f;
  float qt_note_t = 1.0f;

  float eight_note_d = 1.0f;
  float eight_note =1.0f;
  float eight_note_t = 1.0f;

  float sixth_note_d =1.0f;
  float sixth_note = 1.0f;
  float sixth_note_t = 1.0f;

  float trth_note_d = 1.0f;
  float trth_note = 1.0f;
  float trth_note_t = 1.0f;
  //hz variables
  float hz_bar = 1.0f;
  float half_hz_d = 1.0f;
  float half_hz = 1.0f;
  float half_hz_t = 1.0f;

  float qt_hz_d = 1.0f;
  float qt_hz = 1.0f;
  float qt_hz_t = 1.0f;

  float eight_hz_d = 1.0f;
  float eight_hz = 1.0f;
  float eight_hz_t = 1.0f;

  float sixth_hz_d = 1.0f;
  float sixth_hz = 1.0f;
  float sixth_hz_t = 1.0f;

  float trth_hz_d = 1.0f;
  float trth_hz = 1.0f;
  float trth_hz_t = 1.0f;


  void calculateValues(float bpm){ 

        millisecondsPerBeat = millisecs/bpm;
        millisecondsPerMeasure = millisecondsPerBeat * 4;

        secondsPerBeat = 60 / bpm;
        secondsPerMeasure = secondsPerBeat * 4;

        bar = (millisecondsPerMeasure);

        half_note_d = ( millisecondsPerBeat * 3 );
        half_note = ( millisecondsPerBeat * 2 );
        half_note_t = ( millisecondsPerBeat * 2 * 2 / 3 );

        qt_note_d = ( millisecondsPerBeat / 2 ) * 3;
        qt_note = millisecondsPerBeat;
        qt_note_t = ( millisecondsPerBeat * 2 ) / 3;

        eight_note_d = ( millisecondsPerBeat / 4 ) * 3;
        eight_note = millisecondsPerBeat / 2;
        eight_note_t = millisecondsPerBeat / 3;

        sixth_note_d = ( millisecondsPerBeat / 4 ) * 1.5;
        sixth_note = millisecondsPerBeat / 4;
        sixth_note_t = millisecondsPerBeat / 6;

        trth_note_d = ( millisecondsPerBeat / 8 ) * 1.5;
        trth_note = millisecondsPerBeat / 8;
        trth_note_t = millisecondsPerBeat / 8 * 2 / 3;
        //hz measures
        hz_bar = (1/secondsPerMeasure);

        half_hz_d = mult / half_note_d;
        half_hz = mult / half_note;
        half_hz_t = mult / half_note_t;

        qt_hz_d = mult / qt_note_d;
        qt_hz = mult / qt_note;
        qt_hz_t = mult / qt_note_t;

        eight_hz_d = mult / eight_note_d;
        eight_hz = mult / eight_note;
        eight_hz_t = mult / eight_note_t;

        sixth_hz_d = mult / sixth_note_d;
        sixth_hz = mult / sixth_note;
        sixth_hz_t = mult / sixth_note_t;

        trth_hz_d = mult / trth_note_d;
        trth_hz = mult / trth_note;
        trth_hz_t = mult / trth_note_t;

         last_bpm = bpm;
        //seems like round calcs are not really needed:
        /*
        half_note_d = std::round(millisecondsPerBeat * 3 * mult)/mult;
        half_note = std::round(millisecondsPerBeat * 2 * mult)/mult;
        half_note_t = std::round(millisecondsPerBeat * 2 * 2 / 3 * mult)/mult;
        */
  }

  void refreshDetector() {

      inMemory = false;
      beatLock = false;
      beatTime = 0.0f;
      beatCount = 0;
      beatCountMemory = 0;
      beatOld = 0.0f;
      tempo = "---";
      pulse = false;

	}

  
	BPMCalc2() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(BPMCalc2::TEMPO_PARAM, 30.0f, 300.0f, 120.0f, "Tempo");
  }

  void onReset() override {

    refreshDetector();

	}

	void onInitialize() {

    refreshDetector();

	}

  void process(const ProcessArgs &args) override {

    //BPM detection code
    float deltaTime = args.sampleTime;

    if ( inputs[CLOCK_INPUT].isConnected() ) {

      float clockInput = inputs[CLOCK_INPUT].getVoltage();
      //A rising slope
      if ( ( clockTrigger.process( inputs[CLOCK_INPUT].getVoltage() ) ) && !inMemory ) {
        beatCount++;
        if(!beatLock){
          lights[CLOCK_LIGHT].value = 1.0f;
          LightPulse.trigger( 0.1f );
        }
        inMemory = true;
      
        //BPM is locked
        if ( beatCount == 2 ) {
          lights[CLOCK_LOCK_LIGHT].value = 1.0f;
          beatLock = true;
          beatOld = beatTime;
        }
        //BPM is lost
        if ( beatCount > 2 ) {

          if ( fabs( beatOld - beatTime ) > 0.0005f ) {
            beatLock = false;
            beatCount = 0;
            lights[CLOCK_LOCK_LIGHT].value = 0.0f;
            tempo = "---";
          }

        }

        beatTime = 0;

      }

      //Falling slope
      if ( clockInput <= 0 && inMemory ) {
        inMemory = false;
      }
      //When BPM is locked
      if ( beatLock ) {
        bpm = (int)round( 60 / beatOld );
        tempo = std::to_string( (int)round(bpm) );
        if(bpm!=last_bpm){
          if(bpm<999){
            calculateValues(bpm);
          }else{
            tempo = "OOR";
          }
        }
    
      } //end of beatLock routine

      beatTime += deltaTime;

      //when beat is lost
      if ( beatTime > 2 ) {
        beatLock = false;
        beatCount = 0;
        lights[CLOCK_LOCK_LIGHT].value = 0.0f;
        tempo = "---";
      }
      beatCountMemory = beatCount;

    } else {
      beatLock = false;
      beatCount = 0;
      //tempo = "OFF";
      lights[CLOCK_LOCK_LIGHT].value = 0.0f;
      //caluculate with knob value instead of bmp detector value
      bpm = params[TEMPO_PARAM].getValue();
      if (bpm<30){
        bpm = 30;
      }
      bpm = (int)round(bpm);
      tempo = std::to_string( (int)round(bpm) );
      if(bpm!=last_bpm){
          calculateValues(bpm);
      }

    }

    pulse = LightPulse.process( 1.0 / args.sampleRate );
    lights[CLOCK_LIGHT].value = (pulse ? 1.0f : 0.0f);

    //OUTPUTS: MS to 10V scaled values
    outputs[MS_OUTPUT+0].setVoltage(rescale(bar,0.0f,10000.0f,0.0f,10.0f));
    outputs[MS_OUTPUT+1].setVoltage(rescale(half_note_d,0.0f,10000.0f,0.0f,10.0f));

    outputs[MS_OUTPUT+2].setVoltage(rescale(half_note,0.0f,10000.0f,0.0f,10.0f));
    outputs[MS_OUTPUT+3].setVoltage(rescale(half_note_t,0.0f,10000.0f,0.0f,10.0f));

    outputs[MS_OUTPUT+4].setVoltage(rescale(qt_note_d,0.0f,10000.0f,0.0f,10.0f));
    outputs[MS_OUTPUT+5].setVoltage(rescale(qt_note,0.0f,10000.0f,0.0f,10.0f));

    outputs[MS_OUTPUT+6].setVoltage(rescale(qt_note_t,0.0f,10000.0f,0.0f,10.0f));
    outputs[MS_OUTPUT+7].setVoltage(rescale(eight_note_d,0.0f,10000.0f,0.0f,10.0f));

    outputs[MS_OUTPUT+8].setVoltage(rescale(eight_note,0.0f,10000.0f,0.0f,10.0f));
    outputs[MS_OUTPUT+9].setVoltage(rescale(eight_note_t,0.0f,10000.0f,0.0f,10.0f));

    outputs[MS_OUTPUT+10].setVoltage(rescale(sixth_note_d,0.0f,10000.0f,0.0f,10.0f));
    outputs[MS_OUTPUT+11].setVoltage(rescale(sixth_note,0.0f,10000.0f,0.0f,10.0f));

    outputs[MS_OUTPUT+12].setVoltage(rescale(sixth_note_t,0.0f,10000.0f,0.0f,10.0f));
    outputs[MS_OUTPUT+13].setVoltage(rescale(trth_note_d,0.0f,10000.0f,0.0f,10.0f));

    outputs[MS_OUTPUT+14].setVoltage(rescale(trth_note,0.0f,10000.0f,0.0f,10.0f));
    
    outputs[MS_OUTPUT+15].setVoltage(rescale(trth_note_t,0.0f,10000.0f,0.0f,10.0f));

  }

};

////////////////////////////////////
struct TempodisplayWidget : TransparentWidget {
 std::string *value = NULL;
  std::shared_ptr<Font> font;

  TempodisplayWidget() {
    font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Segment7Standard.ttf"));
  };

  void draw(const DrawArgs &args) override{
    if (!value) {
      return;
    }
    // Background
    /*
    NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(args.vg, backgroundColor);
    nvgFill(args.vg);
    nvgStrokeWidth(args.vg, 1.5);
    nvgStrokeColor(args.vg, borderColor);
    nvgStroke(args.vg); 
    */   
    // text 
    nvgFontSize(args.vg, 18);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 2.5);

    std::stringstream to_display;   
    to_display << std::setw(3) << *value;

    Vec textPos = Vec(14.0f, 17.0f); 
    /*
    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(args.vg, nvgTransRGBA(textColor, 16));
    nvgText(args.vg, textPos.x, textPos.y, "~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(args.vg, nvgTransRGBA(textColor, 16));
    nvgText(args.vg, textPos.x, textPos.y, "\\\\\\", NULL);
    */
    NVGcolor textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(args.vg, textColor);
    nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};


//////////////////////////////////

struct BPMCalc2Widget : ModuleWidget {

  BPMCalc2Widget(BPMCalc2 *module) {

    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BPMCalc2.svg")));

    //SCREWS
    addChild(createWidget<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    //BPM DETECTOR PORT
    addInput(createInput<as_PJ301MPort>(Vec(7, 53), module, BPMCalc2::CLOCK_INPUT));

    //BPM DISPLAY 
    TempodisplayWidget *display = new TempodisplayWidget();
    display->box.pos = Vec(55,54);
    display->box.size = Vec(55, 20);
    if (module) {
      display->value = &module->tempo;
    }
    addChild(display);
    //DETECTOR LEDS
    addChild(createLight<DisplayLedLight<RedLight>>(Vec(57, 56), module, BPMCalc2::CLOCK_LOCK_LIGHT));
    addChild(createLight<DisplayLedLight<RedLight>>(Vec(57, 66), module, BPMCalc2::CLOCK_LIGHT)); 
    //TEMPO KNOB
    addParam(createParam<as_KnobBlack>(Vec(45, 84), module, BPMCalc2::TEMPO_PARAM));

    //MS outputs
    int const out_offset = 40;
    // 1 
    addOutput(createOutput<as_PJ301MPort>(Vec(84, 126), module, BPMCalc2::MS_OUTPUT + 0));
    //·1/2 - 1/2 - t1/2
    addOutput(createOutput<as_PJ301MPort>(Vec(8, 126+out_offset*1), module, BPMCalc2::MS_OUTPUT + 1));
    addOutput(createOutput<as_PJ301MPort>(Vec(48, 126+out_offset*1), module, BPMCalc2::MS_OUTPUT + 2));
    addOutput(createOutput<as_PJ301MPort>(Vec(84, 126+out_offset*1), module, BPMCalc2::MS_OUTPUT + 3));
    // ·1/4 - 1/4 -  t1/4
    addOutput(createOutput<as_PJ301MPort>(Vec(8, 126+out_offset*2), module, BPMCalc2::MS_OUTPUT + 4));
    addOutput(createOutput<as_PJ301MPort>(Vec(48, 126+out_offset*2), module, BPMCalc2::MS_OUTPUT + 5));
    addOutput(createOutput<as_PJ301MPort>(Vec(84, 126+out_offset*2), module, BPMCalc2::MS_OUTPUT + 6));

    // ·1/8 - 1/8 - t1/8
  addOutput(createOutput<as_PJ301MPort>(Vec(8, 126+out_offset*3), module, BPMCalc2::MS_OUTPUT + 7));
    addOutput(createOutput<as_PJ301MPort>(Vec(48, 126+out_offset*3), module, BPMCalc2::MS_OUTPUT + 8));
    addOutput(createOutput<as_PJ301MPort>(Vec(84, 126+out_offset*3), module, BPMCalc2::MS_OUTPUT + 9));
    // ·1/16 - 1/16
    addOutput(createOutput<as_PJ301MPort>(Vec(8, 126+out_offset*4), module, BPMCalc2::MS_OUTPUT + 10));
    addOutput(createOutput<as_PJ301MPort>(Vec(48, 126+out_offset*4), module, BPMCalc2::MS_OUTPUT + 11));

    // t1/16 - ·1/32
    addOutput(createOutput<as_PJ301MPort>(Vec(84, 126+out_offset*4), module, BPMCalc2::MS_OUTPUT + 12));
    addOutput(createOutput<as_PJ301MPort>(Vec(8, 126+out_offset*5), module, BPMCalc2::MS_OUTPUT + 13));
    
    // 1/32 - t1/32
    addOutput(createOutput<as_PJ301MPort>(Vec(48, 126+out_offset*5), module, BPMCalc2::MS_OUTPUT + 14));
    addOutput(createOutput<as_PJ301MPort>(Vec(84, 126+out_offset*5), module, BPMCalc2::MS_OUTPUT + 15));
    
  }
};


Model *modelBPMCalc2 = createModel<BPMCalc2, BPMCalc2Widget>("BPMCalc2");
