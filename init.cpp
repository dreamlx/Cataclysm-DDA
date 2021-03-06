#include "init.h"

#include "json.h"

#include "material.h"
#include "bionics.h"
#include "profession.h"
#include "skill.h"
#include "mutation.h"
#include "text_snippets.h"
#include "item_factory.h"
#include "crafting.h"
#include "computer.h"
#include "help.h"
#include "mapdata.h"
#include "color.h"
#include "monstergenerator.h"
#include "inventory.h"
#include "tutorial.h"
#include "artifact.h"

#include <string>
#include <vector>
#include <fstream>
#include <sstream> // for throwing errors

#include "savegame.h"

typedef std::string type_string;
std::map<type_string, TFunctor*> type_function_map;

std::map<int,int> reverse_legacy_ter_id;
std::map<int,int> reverse_legacy_furn_id;
/*
 * Populate optional ter_id and furn_id variables
 */
void init_data_mappings() {
    set_ter_ids();
    set_furn_ids();

// temporary (reliable) kludge until switch statements are rewritten
    std::map<std::string, int> legacy_lookup;
    for(int i=0; i< num_legacy_ter;i++) {
         legacy_lookup[ legacy_ter_id[i] ] = i;
    }
    reverse_legacy_ter_id.clear();
    for( int i=0; i < terlist.size(); i++ ) {
        if ( legacy_lookup.find(terlist[i].id) != legacy_lookup.end() ) {
             reverse_legacy_ter_id[ i ] = legacy_lookup[ terlist[i].id ];
        } else {
             reverse_legacy_ter_id[ i ] = 0;
        }
    }
    legacy_lookup.clear();
    for(int i=0; i< num_legacy_furn;i++) {
         legacy_lookup[ legacy_furn_id[i] ] = i;
    }
    reverse_legacy_furn_id.clear();
    for( int i=0; i < furnlist.size(); i++ ) {
        if ( legacy_lookup.find(furnlist[i].id) != legacy_lookup.end() ) {
             reverse_legacy_furn_id[ i ] = legacy_lookup[ furnlist[i].id ];
        } else {
             reverse_legacy_furn_id[ i ] = 0;
        }
    }
}

/* Currently just for loading JSON data from files in data/raw */

// TODO: make this actually load files from the named directory
std::vector<std::string> listfiles(std::string const &dirname)
{
    (void)dirname; //not used yet
    std::vector<std::string> ret;

    ret.push_back("data/json/materials.json");
    ret.push_back("data/json/bionics.json");
    ret.push_back("data/json/professions.json");
    ret.push_back("data/json/skills.json");
    ret.push_back("data/json/dreams.json");
    ret.push_back("data/json/mutations.json");
    ret.push_back("data/json/snippets.json");
    ret.push_back("data/json/item_groups.json");
    ret.push_back("data/json/lab_notes.json");
    ret.push_back("data/json/hints.json");
    ret.push_back("data/json/furniture.json");
    ret.push_back("data/json/terrain.json");
    ret.push_back("data/json/migo_speech.json");
    ret.push_back("data/json/names.json");
    ret.push_back("data/json/vehicle_parts.json");
    ret.push_back("data/json/vehicles.json");
    ret.push_back("data/json/species.json");
    ret.push_back("data/json/monsters.json");
    ret.push_back("data/json/monstergroups.json");
    ret.push_back("data/json/items/ammo.json");
    ret.push_back("data/json/items/archery.json");
    ret.push_back("data/json/items/armor.json");
    ret.push_back("data/json/items/books.json");
    ret.push_back("data/json/items/comestibles.json");
    ret.push_back("data/json/items/containers.json");
    ret.push_back("data/json/items/melee.json");
    ret.push_back("data/json/items/mods.json");
    ret.push_back("data/json/items/ranged.json");
    ret.push_back("data/json/items/tools.json");
    ret.push_back("data/json/items/vehicle_parts.json");
    ret.push_back("data/json/techniques.json");
    ret.push_back("data/json/martialarts.json");
    ret.push_back("data/json/tutorial.json");
    ret.push_back("data/json/tool_qualities.json");
    ret.push_back("data/json/recipes.json");

    return ret;
}

void load_object(JsonObject &jo)
{
    std::string type = jo.get_string("type");
    if (type_function_map.find(type) != type_function_map.end())
    {
        (*type_function_map[type])(jo);
    } else {
        std::stringstream err;
        err << jo.line_number() << ": ";
        err << "unrecognized JSON object, type: \"" << type << "\"";
        throw err.str();
    }
}

void null_load_target(JsonObject &) {}

void init_data_structures()
{
    // all of the applicable types that can be loaded, along with their loading functions
    // Add to this as needed with new StaticFunctionAccessors or new ClassFunctionAccessors for new applicable types
    // Static Function Access
    type_function_map["material"] = new StaticFunctionAccessor(&material_type::load_material);
    type_function_map["bionic"] = new StaticFunctionAccessor(&load_bionic);
    type_function_map["profession"] = new StaticFunctionAccessor(&profession::load_profession);
    type_function_map["skill"] = new StaticFunctionAccessor(&Skill::load_skill);
    type_function_map["dream"] = new StaticFunctionAccessor(&load_dream);
    type_function_map["mutation"] = new StaticFunctionAccessor(&load_mutation);
    type_function_map["lab_note"] = new StaticFunctionAccessor(&computer::load_lab_note);
    type_function_map["hint"] = new StaticFunctionAccessor(&load_hint);
    type_function_map["furniture"] = new StaticFunctionAccessor(&load_furniture);
    type_function_map["terrain"] = new StaticFunctionAccessor(&load_terrain);
    type_function_map["monstergroup"] = new StaticFunctionAccessor(&MonsterGroupManager::LoadMonsterGroup);
    //data/json/colors.json would be listed here, but it's loaded before the others (see curses_start_color())

    // Non Static Function Access
    type_function_map["snippet"] = new ClassFunctionAccessor<snippet_library>(&SNIPPET, &snippet_library::load_snippet);
    type_function_map["item_group"] = new ClassFunctionAccessor<Item_factory>(item_controller, &Item_factory::load_item_group);
    type_function_map["migo_speech"] = new ClassFunctionAccessor<game>(g, &game::load_migo_speech);
    type_function_map["NAME"] = new ClassFunctionAccessor<NameGenerator>(&NameGenerator::generator(), &NameGenerator::load_name);

    type_function_map["vehicle_part"] = new ClassFunctionAccessor<game>(g, &game::load_vehiclepart);
    type_function_map["vehicle"] = new ClassFunctionAccessor<game>(g, &game::load_vehicle);
    type_function_map["AMMO"] = new ClassFunctionAccessor<Item_factory>(item_controller, &Item_factory::load_ammo);
    type_function_map["GUN"] = new ClassFunctionAccessor<Item_factory>(item_controller, &Item_factory::load_gun);
    type_function_map["ARMOR"] = new ClassFunctionAccessor<Item_factory>(item_controller, &Item_factory::load_armor);
    type_function_map["TOOL"] = new ClassFunctionAccessor<Item_factory>(item_controller, &Item_factory::load_tool);
    type_function_map["BOOK"] = new ClassFunctionAccessor<Item_factory>(item_controller, &Item_factory::load_book);
    type_function_map["COMESTIBLE"] = new ClassFunctionAccessor<Item_factory>(item_controller, &Item_factory::load_comestible);
    type_function_map["CONTAINER"] = new ClassFunctionAccessor<Item_factory>(item_controller, &Item_factory::load_container);
    type_function_map["GUNMOD"] = new ClassFunctionAccessor<Item_factory>(item_controller, &Item_factory::load_gunmod);
    type_function_map["GENERIC"] = new ClassFunctionAccessor<Item_factory>(item_controller, &Item_factory::load_generic);

    type_function_map["MONSTER"] = new ClassFunctionAccessor<MonsterGenerator>(&MonsterGenerator::generator(), &MonsterGenerator::load_monster);
    type_function_map["SPECIES"] = new ClassFunctionAccessor<MonsterGenerator>(&MonsterGenerator::generator(), &MonsterGenerator::load_species);

    type_function_map["recipe_category"] = new StaticFunctionAccessor(&load_recipe_category);
    type_function_map["recipe"] = new StaticFunctionAccessor(&load_recipe);
    type_function_map["tool_quality"] = new StaticFunctionAccessor(&load_quality);
    type_function_map["technique"] = new StaticFunctionAccessor(&load_technique);
    type_function_map["martial_art"] = new StaticFunctionAccessor(&load_martial_art);
    type_function_map["tutorial_messages"] =
        new StaticFunctionAccessor(&load_tutorial_messages);

    mutations_category[""].clear();
    init_mutation_parts();
    init_translation();
    init_martial_arts();
    init_inventory_categories();
    init_artifacts();
}

void release_data_structures()
{
    std::map<type_string, TFunctor*>::iterator it;
    for (it = type_function_map.begin(); it != type_function_map.end(); it++) {
        if (it->second != NULL)
            delete it->second;
    }
    type_function_map.clear();
}

void load_json_dir(std::string const &dirname)
{
    // get a list of all files in the directory
    std::vector<std::string> dir = listfiles(dirname);
    // iterate over each file
    std::vector<std::string>::iterator it;
    for (it = dir.begin(); it != dir.end(); it++) {
        // open the file as a stream
        std::ifstream infile(it->c_str(), std::ifstream::in | std::ifstream::binary);
        // and stuff it into ram
        std::istringstream iss(
            std::string(
                (std::istreambuf_iterator<char>(infile)),
                std::istreambuf_iterator<char>()
            )
        );
        infile.close();
        // parse it
        try {
            JsonIn jsin(&iss);
            load_all_from_json(jsin);
        } catch (std::string e) {
            throw *(it) + ": " + e;
        }
    }
    init_data_mappings();
}

void load_all_from_json(JsonIn &jsin)
{
    char ch;
    std::string type = "";
    jsin.eat_whitespace();
    // examine first non-whitespace char
    ch = jsin.peek();
    if (ch == '{') {
        // find type and dispatch single object
        JsonObject jo = jsin.get_object();
        load_object(jo);
        jo.finish();
        // if there's anything else in the file, it's an error.
        jsin.eat_whitespace();
        if (jsin.good()) {
            std::stringstream err;
            err << jsin.line_number() << ": ";
            err << "expected single-object file but found '";
            err << jsin.peek() << "'";
            throw err.str();
        }
    } else if (ch == '[') {
        jsin.start_array();
        // find type and dispatch each object until array close
        while (!jsin.end_array()) {
            jsin.eat_whitespace();
            ch = jsin.peek();
            if (ch != '{') {
                std::stringstream err;
                err << jsin.line_number() << ": ";
                err << "expected array of objects but found '";
                err << ch << "', not '{'";
                throw err.str();
            }
            JsonObject jo = jsin.get_object();
            load_object(jo);
            jo.finish();
        }
    } else {
        // not an object or an array?
        std::stringstream err;
        err << jsin.line_number() << ": ";
        err << "expected object or array, but found '" << ch << "'";
        throw err.str();
    }
}


