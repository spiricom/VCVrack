#include "Tutorial.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	// The "slug" is the unique identifier for your plugin and must never change after release, so choose wisely.
	// It must only contain letters, numbers, and characters "-" and "_". No spaces.
	// To guarantee uniqueness, it is a good idea to prefix the slug by your name, alias, or company name if available, e.g. "MyCompany-MyPlugin".
	// The ZIP package must only contain one folder, with the name equal to the plugin's slug.
	p->slug = "Tutorial";
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/VCVRack/Tutorial";
	p->manual = "https://github.com/VCVRack/Tutorial/blob/master/README.md";

	// For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
	p->addModel(createModel<MyModuleWidget>("Tutorial", "MyModule", "My Module", OSCILLATOR_TAG));

	p->addModel(createModel<OOPSModuleWidget>("Tutorial", "OOPSModule", "OOPS Module", OSCILLATOR_TAG));

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}