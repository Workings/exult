/*
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2013  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef INCL_EGG
#define INCL_EGG    1

class   Egg_object;
class   Animator;
class   Monster_actor;
class   Missile_launcher;

#include "iregobjs.h"
#include "ignore_unused_variable_warning.h"

/*
 *  Here's a class for eggs and paths; i.e., objects that generally aren't
 *  visible.
 */
class Egglike_game_object : public Ireg_game_object {
public:
	Egglike_game_object(int shapenum, int framenum,
	                    unsigned int tilex,
	                    unsigned int tiley, unsigned int lft = 0)
		: Ireg_game_object(shapenum, framenum, tilex, tiley, lft)
	{  }
	virtual ~Egglike_game_object() {  }
	// Render.
	virtual void paint();
	// Can this be clicked on?
	virtual int is_findable();
};

/*
 *  An "egg" is a special object that activates under certain
 *  circumstances.
 */
class Egg_object : public Egglike_game_object {
	static Egg_object *editing; // Egg being edited by ExultStudio.
protected:
	unsigned char type;     // One of the below types.
	unsigned char probability;  // 1-100, chance of egg activating.
	unsigned char criteria: 3;  // How it's activated.  See below.
	unsigned distance: 6;       // Distance for activation (0-31).
	unsigned flags: 4;      // Formed from below flags.
	unsigned short data1, data2, data3; // More data, dep. on type.
	Rectangle area;         // Active area.
	unsigned char solid_area;   // 1 if area is solid, 0 if outline.
	Animator *animator;     // Controls animation.
	void init_field(unsigned char ty);
	static Egg_object *create_egg(bool animated,
	                              int shnum, int frnum, unsigned int tx,
	                              unsigned int ty, unsigned int tz,
	                              unsigned short itype,
	                              unsigned char prob, short data1, short data2, short data3,
	                              const char *str1 = 0);
public:
	friend class Button_egg;
	enum Egg_types {        // Types of eggs:
	    monster = 1,
	    jukebox = 2,
	    soundsfx = 3,
	    voice = 4,
	    usecode = 5,
	    missile = 6,
	    teleport = 7,
	    weather = 8,
	    path = 9,
	    button = 10,
	    intermap = 11,
	    // Our own:
	    fire_field = 128,
	    sleep_field = 129,
	    poison_field = 130,
	    caltrops_field = 131,
		campfire_field = 132,
	    mirror_object = 133
	};
	enum Egg_flag_shifts {
	    nocturnal = 0,
	    once = 1,
	    hatched = 2,
	    auto_reset = 3
	};
	enum Egg_criteria {
	    cached_in = 0,      // Activated when chunk read in?
	    party_near = 1,
	    avatar_near = 2,    // Avatar steps into area.
	    avatar_far = 3,     // Avatar steps outside area.
	    avatar_footpad = 4, // Avatar must step on it.
	    party_footpad = 5,
	    something_on = 6,   // Something placed on/near it.
	    external_criteria = 7   // Appears on Isle of Avatar.  Guessing
	                        //   these set off all nearby.
	};
	static Egg_object *create_egg(unsigned char *entry, int entlen,
	                              bool animated, int shnum, int frnum, unsigned int tx,
	                              unsigned int ty, unsigned int tz);
	// Create normal eggs.
	Egg_object(int shapenum, int framenum, unsigned int tilex,
	           unsigned int tiley, unsigned int lft,
	           unsigned short itype,
	           unsigned char prob, short d1, short d2, short d3 = 0);
	// Ctor. for fields:
	Egg_object(int shapenum, int framenum, unsigned int tilex,
	           unsigned int tiley, unsigned int lft,
	           unsigned char ty);
	virtual ~Egg_object();
	virtual void set_area();        // Set up active area.
	int get_distance() const {
		return distance;
	}
	int get_criteria() const {
		return criteria;
	}
	int get_type() const {
		return type;
	}
	virtual const char *get_str1() {
		return "";
	}
	virtual void set_str1(const char *s) {
		ignore_unused_variable_warning(s);
	}
	// Can this be clicked on?
	virtual int is_findable();
	virtual void set(int crit, int dist);
	// Can it be activated?
	virtual int is_active(Game_object *obj,
	                      int tx, int ty, int tz, int from_tx, int from_ty);

	Rectangle get_area() const { // Get active area.
		return area;
	}
	int is_solid_area() const {
		return solid_area;
	}
	void set_animator(Animator *a);
	void stop_animation();
	virtual void paint();
	// Run usecode function.
	virtual void activate(int event = 1);
	virtual bool edit();        // Edit in ExultStudio.
	// Saved from ExultStudio.
	static void update_from_studio(unsigned char *data, int datalen);
	virtual void hatch_now(Game_object *obj, bool must) {
		ignore_unused_variable_warning(obj, must);
	}
	virtual void hatch(Game_object *obj, bool must = false);
	void print_debug();
	static void set_weather(int weather, int len = 15,
	                        Game_object *egg = 0);
	// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift, int newmap = -1);
	// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual int is_egg() const { // An egg?
		return 1;
	}
	// Write out to IREG file.
	virtual void write_ireg(DataSource *out);
	// Get size of IREG. Returns -1 if can't write to buffer
	virtual int get_ireg_size();

	virtual void reset() {
		flags &= ~(1 << hatched);
	}

	virtual Egg_object *as_egg() {
		return this;
	}

};


/*
 *  Fields are activated like eggs.
 */

class Field_object : public Egg_object {
	bool field_effect(Actor *actor);// Apply field.
public:
	Field_object(int shapenum, int framenum, unsigned int tilex,
	             unsigned int tiley, unsigned int lft, unsigned char ty);
	virtual void paint();
	// Run usecode function.
	virtual void activate(int event = 1);
	virtual void hatch(Game_object *obj, bool must = false);
	// Write out to IREG file.
	virtual void write_ireg(DataSource *out);
	// Get size of IREG. Returns -1 if can't write to buffer
	virtual int get_ireg_size();
	virtual int is_findable() {
		return Ireg_game_object::is_findable();
	}
	virtual bool edit() {
		return Ireg_game_object::edit();
	}
	static void update_from_studio(unsigned char *data, int datalen) {
		Ireg_game_object::update_from_studio(data, datalen);
	}
};

/*
 *  Mirrors are handled like eggs.
 */

class Mirror_object : public Egg_object {
public:
	Mirror_object(int shapenum, int framenum, unsigned int tilex,
	              unsigned int tiley, unsigned int lft);

	// Run usecode function.
	virtual void activate(int event = 1);
	virtual void hatch(Game_object *obj, bool must = false);

	// Can it be activated?
	virtual int is_active(Game_object *obj,
	                      int tx, int ty, int tz, int from_tx, int from_ty);

	virtual void set_area();        // Set up active area.

	// Render.
	virtual void paint();
	// Can this be clicked on?
	virtual int is_findable() {
		return Ireg_game_object::is_findable();
	}

	virtual void write_ireg(DataSource *out);
	// Get size of IREG. Returns -1 if can't write to buffer
	virtual int get_ireg_size();
	virtual bool edit() {
		return Ireg_game_object::edit();
	}
	static void update_from_studio(unsigned char *data, int datalen) {
		Ireg_game_object::update_from_studio(data, datalen);
	}
};
#endif
