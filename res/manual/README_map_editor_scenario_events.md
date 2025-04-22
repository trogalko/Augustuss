# Assignment Editor - Scenario events

This readme is meant for map makers to get a better understanding of how to use the new scenario events,
and write the XML needed to import it.


## What are scenario events?

Scenario events is a unique feature added to Augustus that allows map makers to do all sorts of
interesting things.

Examples would be things like:
+ 'If the player is playing on very hard' and 'the scenario has just started' then 'remove 2500 money from their city funds'
+ 'If 17 months have passed' then 'decrease the trade price of pottery by 10'
+ 'Every 6 months' if 'the player has 70 or more favour' and 'Rome buys less than 100 wine per year' then 'increase the trade demand of wine for Rome by +5'
+ 'Every month' if 'the player has 60 or less favour' and 'Rome buys more than 15 wine per year' then 'decrease the trade demand of wine for Rome by -1'

In general a scenario event is one or more conditions, that if they are all true, will take one or more actions.


## How do I add scenario events to my map?

Scenario events are imported with an XML file. Once you have the file the importing is easy.
We'll cover making an XML file a bit later.

To import:
+ In the assignment editor, load the map you want to import scenario events for.
+ Open the scenario details window (button below the minimap).
+ You'll see a button for 'Scenario Events'.
+ Click that button.
+ You'll see a new window that shows you info on the scenario events. (if there are any).
+ At the bottom there are two buttons, one for import and one for export.
+ Click the import button.
+ A file selection window will open.
+ It will list all the files that are in your './editor/events' folder. (A folder called 'editor' inside your Caesar 3 folder, and a folder called 'events' inside that.)
+ Select the file you want to import.
+ Now click proceed. (The tick box.)
+ The scenario events should update to reflect the events you just imported.


## There is a map with interesting events on it. How can I see how the events are setup?
+ In the assignment editor, load the map you want to import scenario events for.
+ Open the scenario details window (button below the minimap).
+ You'll see a button for 'Scenario Events'.
+ Click that button.

If this is a new map (or one that doesn't have any events) then you'll see the number shows 0, and there is no list.
If there are already events in the map, you'll see a list of the events and how many conditions and actions each one has.

+ When looking at the window that shows you info on the scenario events.
+ There is a scrollable list that shows summary info on the events.
+ Click an event to get more details on that event, its conditions, and actions.
+ The new window will show you info for that specific event.
+ Firstly it will show you if this event can repeat.
+ Next is a list showing conditions and actions.
+ Conditions are in red, and the type of condition and its parameters are shown.
+ Actions are next, and their type of action and parameters are shown.


+ When looking at the window that shows you info on the scenario events.
+ At the bottom, click the export button.
+ A file selection / save window appears.
+ Type a new file name to save the exported events XML as.
+ Alternatively, pick an existing file (BE CAREFUL! This will override that file!)
+ Click the proceed button. (The tick box.)
+ Outside of Caesar 3, go to the './evemts' folder (a folder inside your Caesar 3 folder).
+ Open the exported file with a text editor. (Notepad++ or something similar.)
+ You can see exactly how the scenario events were setup.
+ This is also a valid XML file, you can import this again into a map.


## How often are events checked and ran?

Every time the month ticks over (changes to the next month).


## Event settings

There are a few settings we can set on the event itself.

Making the event repeat
+ "repeat_months_min" = If this is greater than 0, and max is equal or greater than min, then this event will repeat (it becomes paused instead of disabled after running.)
+ "repeat_months_max" = The event will become active again after a random number of months between min and max (if the two are equal, then the number is not random).
+ "max_number_of_repeats" = This is the maximum number of times the event is allowed to trigger. Settings this to 0 will allow the event to trigger infinitely.


## Available conditions

Here is a list of available conditions that can be used.
+ "type =" is what it will be called in the XML.
+ "Allowed values: " below shows you what values are allowed for that setting.


### Building active count
+ type = building_count_active
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 10000000.
+ building = Which type of building to cound.
  - Allowed values: A very big list. Easiest is look at this condition type in the editor.
    Otherwise, refer to: (special_attribute_mappings_buildings in "https://github.com/Keriew/augustus/blob/9429b006f38873749e4b4aa3c0cee99696eb23b2/src/scenario/scenario_events_parameter_data.c#L234")
  - Some notes:
    - "all_farms" = will count all farm types.
    - "all_raw_materials" = will count all marble quarries, clay pits, timber yards, iron and gold mines.
    - "all_workshops" = will count all workshops.
    - "all_small_temples" = will count all small temples.
    - "all_large_temples" = will count all large temples.
    - "all_grand_temples" = will count all grand temples. (Excluding pantheon)
    - "all_trees" = will count all decorative trees.
    - "all_paths" = will count all decorative paths. (Including garden paths.)
    - "all_parks" = will count all gardens, statues and ponds.


### Building count (inactive or active)
+ type = building_count_any
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 10000000.
+ building = Which type of building to cound.
  - Allowed values: A very big list. Easiest is look at this condition type in the editor.
    Otherwise, refer to: (special_attribute_mappings_buildings in "https://github.com/Keriew/augustus/blob/9429b006f38873749e4b4aa3c0cee99696eb23b2/src/scenario/scenario_events_parameter_data.c#L234")
  - Some notes:
    - "all_farms" = will count all farm types.
    - "all_raw_materials" = will count all marble quarries, clay pits, timber yards, iron and gold mines.
    - "all_workshops" = will count all workshops.
    - "all_small_temples" = will count all small temples.
    - "all_large_temples" = will count all large temples.
    - "all_grand_temples" = will count all grand temples. (Excluding pantheon)
    - "all_trees" = will count all decorative trees.
    - "all_paths" = will count all decorative paths. (Including garden paths.)
    - "all_parks" = will count all gardens, statues and ponds.


### Building area
+ type = building_count_area
+ grid_offset = The grid_offset (map position) that is the center of the square to collapse buildings in.
+ block_radius = The 'radius' of the square to check buildings in. A radius of 2 will give you a 5x5 square.
+ building = Which type of building to cound.
  - Allowed values: A very big list. Easiest is look at this condition type in the editor.
    Otherwise, refer to: (special_attribute_mappings_buildings in "https://github.com/Keriew/augustus/blob/9429b006f38873749e4b4aa3c0cee99696eb23b2/src/scenario/scenario_events_parameter_data.c#L234")
  - Some notes:
    - "all_farms" = will count all farm types.
    - "all_raw_materials" = will count all marble quarries, clay pits, timber yards, iron and gold mines.
    - "all_workshops" = will count all workshops.
    - "all_small_temples" = will count all small temples.
    - "all_large_temples" = will count all large temples.
    - "all_grand_temples" = will count all grand temples. (Excluding pantheon)
    - "all_trees" = will count all decorative trees.
    - "all_paths" = will count all decorative paths. (Including garden paths.)
    - "all_parks" = will count all gardens, statues and ponds.
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 10000000.


### Count own troops
+ type = count_own_troops
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 100.
+ in_city_only = Should it only count soldiers that are in the city? Allowed values: "false" or "true".
  - "false" will also count soldiers that are away on empire missions.
  - "true" will only count soldiers that are currently in the city.


### Custom variable value
+ type = variable_check
  - Note: To import custom variable names via the events file, use "<variable uid="Myvariablename" initial_value="0"/>" entries in the variables section at the top of the events file. You can have a maximum of 100 different custom variables.
+ variable_uid = The unique name (uid) of the custom variable to use.
  - Allowed values: Any uid of any custom variable that you have imported / set up via the editor.
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: -1000000000 to 1000000000.


### Favor
+ type = stats_favor
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 100.


### Funds: City coffers
+ type = money
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from -10000 to 1000000000.


### Funds: Personal savings
+ type = savings
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 1000000000.


### Game difficulty
+ type = difficulty
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The difficulty to check for. Allowed values: "very_easy","easy","normal","hard","very_hard"


### Months passed
+ type = time
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ min = The minimum amount of months to use for the check. Allowed values: Any number of 0 or more.
+ max = The maximum amount of months to use for the check. Allowed values: Any number of "min" or more.


### Population: City totals
+ type = city_population
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 10000000.
+ class = Which class of pops to count. Allowed values: "all", "patrician", "plebeian", "slums"
  - "all" = Total city population is counted.
  - "patrician" = Only those living in villas or better are counted. WARNING: This stat is only updated once per year.
  - "plebeian" = Only those living in insulae or lesser are counted. WARNING: This stat is only updated once per year.
  - "slums" = Only those living in shacks or tents are counted. WARNING: This stat is only updated once per year.


### Population: Unemployment
+ type = population_unemployed
+ percentage = Should the check do a percentage check? Allowed values: "false" or "true".
  - "false" will compare total unemployed people to value given.
  - "true" will compare % (0 to 100) of unemployed people as % of total population to value given.
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 100.


### Request: Is ongoing request
+ type = request_is_ongoing
+ request_id = Which request to check if it is currently ongoing. Allowed values: 0 to 19
+ check_for_ongoing = Should this condition pass if the request is ongoing? Allowed values: "false" or "true".
  - "false" this condition will pass if the request is not ongoing. I.e. has not yet started, or has been fulfilled or ignored and is gone again.
  - "true" this condition will pass if the request is currently ongoing. I.e. the player can see the request message from Caesar, and still has a chance to fullfill it.


### Resource storage available
+ type = resource_storage_available
+ resource = What resource to use. Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 1000000000.
+ storage_type = What type of storages to check in. Allowed values: "all", "granaries", "warehouses".
  - Note: If you check for non-food types in granaries, the answer will always be 0.
+ respect_settings = Only count empty storage space that is set to accept the target resource. Allowed values: "false" or "true".
  - Note: If false, will count any open storage space, regardless of if the player has set the target resource to be allowed there.
  - Example: A lone warehouse is set to accept up to 8 wheat and 24 wood. And has only 4 wheat stored. If respect_settings = true, the answer is 4. If respect_settings = false, the answer is 28.


### Resource stored count
+ type = resource_stored_count
+ resource = What resource to use. Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 1000000000.
+ storage_type = What type of storages to check in. Allowed values: "all", "granaries", "warehouses".
  - Note: If you check for non-food types in granaries, the answer will always be 0.


### Rome Wages
+ type = rome_wages
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 10000.


### Stats: City Health
+ type = stats_health
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 100.


### Stats: Culture
+ type = stats_culture
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 100.


### Stats: Favor
+ type = stats_favor
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 100.


### Stats: Peace
+ type = stats_peace
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 100.


### Stats: Prosperity
+ type = stats_prosperity
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 100.


### Trade good sell price
+ type = trade_sell_price
+ resource = What resource to use. Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
  - Note: This checks the unadjusted trade price of the target resource. i.e. no caravserai or lighthouse discounts or penalties taken into account.
+ value = The amount to check for. Allowed values: Any number from 0 to 1000000000.


### Trade route is open
+ type = trade_route_open
+ target_city = The name of the city that you want to adjust the route of.
+ check_for_open = Should this condition pass if the trade route is open? Allowed values: "false" or "true".
  - "false" this condition will pass if the route has not yet been opened, or cannot be opened.
  - "true" this condition will pass if the route is open.


### Trade route opening price
+ type = trade_route_price
+ target_city = The name of the city that you want to adjust the route of.
+ check = What sort of check are we doing. Allowed values: "eq", "gte", "lte", "neq", "lt", "gt".
+ value = The amount to check for. Allowed values: Any number from 0 to 1000000000.
  - Note: A trade route that is already open is considered to have a price of 0.




## Available actions

Here is a list of available conditions that can be used.
+ "type =" is what it will be called in the XML.
+ "Allowed values: " below shows you what values are allowed for that setting.


### Allowed buildings
+ type = change_allowed_buildings
+ building = What allowed buildings to change.
  - Allowed values:
    "none", "farms", "raw_materials", "workshops", "road", "wall", "aqueduct", "housing", "amphitheater",
    "theater", "hippodrome", "colosseum", "gladiator_school", "lion_house", "actor_colony", "chariot_maker",
    "gardens", "plaza", "statues", "doctor", "hospital", "bathhouse", "barber", "school", "academy",
    "library", "prefecture", "fort", "gatehouse", "tower", "small_temples", "large_temples", "market",
    "granary", "warehouse", "triumphal_arch", "dock", "wharf", "governor_home", "engineers_post",
    "senate", "forum", "well", "oracle", "mission_post", "bridge", "barracks", "military_academy",
    "monuments"
+ allowed = Should this building type be allowed? Allowed values: "false" or "true".
  - "false" = Building type is disabled and cannot be built.
  - "true" = Building type is enable and avaiable for construction.


### Buildings: Force collapse
+ type = building_force_collapse
+ grid_offset = The grid_offset (map position) that is the center of the square to collapse buildings in.
+ block_radius = The 'radius' of the square to collapse buildings in. A radius of 2 will give you a 5x5 square.
+ building = Which type of building to collapse.
  - Allowed values: A very big list. Easiest is look at this condition type in the editor.
    Otherwise, refer to: (special_attribute_mappings_buildings in "https://github.com/Keriew/augustus/blob/9429b006f38873749e4b4aa3c0cee99696eb23b2/src/scenario/scenario_events_parameter_data.c#L234")
+ destroy_all = Should this collapse all building types, regardless of what is set in building? Allowed values: "false" or "true".
  - "false" = Only collapse buildings that match the type given in building.
  - "true" = Collapse all buildings in the target area.


### Change custom variable
+ type = change_variable
  - Note: To import custom variable names via the events file, use "<variable uid="Myvariablename" initial_value="0"/>" entries in the variables section at the top of the events file. You can have a maximum of 100 different custom variables.
+ variable_uid = The unique name (uid) of the custom variable to use.
  - Allowed values: Any uid of any custom variable that you have imported / set up via the editor.
+ value = The value to change the custom variable by (or set its value to if set_to_value = true). Allowed values: -1000000000 to 1000000000.
+ set_to_value = Should we set the city health to the given value, instead of adding it? Allowed values: "false" or "true".
  - "true" = The custom variable value will be set to what is given in 'value'.
  - "false" = The 'value' will be added to the custom variable value.


### Change resource stockpiles
+ type = change_resource_stockpiles
+ resource = The target resource. Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ amount = The amount to change the stockpile by. Allowed values: Any number from -1000000000 to 1000000000.
  - Note: The value is in full loads.
  - Note: Negative values will remove from the stockpiles. If you try to remove more than there is, it will simply remove as much as possible (so there will be none left).
  - Note: Positive values will add to the stockpiles. If you try to add more than is possible (not  enough free space) it will simply discard the rest (i.e. is lost).
+ storage_type = What type of storages to check in. Allowed values: "all", "granaries", "warehouses".
  - Note: You cannot change the stockpiles of non-food types in granaries. It will simply have no effect.
+ respect_settings = If adding to the stockpiles, only add to places that accept that resource, and only up to the limits set. Allowed values: "false" or "true".
  - Note: If false, and you are adding to the stockpiles, will add the resource to whatever empty space is found.


### City change rating
+ type = change_city_rating
+ rating = The target rating to change. Allowed values: "peace", "prosperity"
+ amount = The amount to adjust the opening price by (or set it to if set_to_value = true). Allowed values: -100 to 100.
  - Note: The rating is limited to 0 to 100.
+ set_to_value = Should we set the target rating to the given value, instead of adding it? Allowed values: "false" or "true".
  - "true" = The target rating will be set to what is given in 'value'.
  - "false" = The 'value' will be added to the current target rating.


### City health
+ type = city_health
+ min = The minimum amount to adjust the city health by. Allowed values: -100 to 100.
+ max = The maximum amount to adjust the city health by. Allowed values: -100 to 100. Must be larger than or equal to 'min'.
+ set_to_value = Should we set the city health to the given value, instead of adding it? Allowed values: "false" or "true".
  - "true" = City health will just be set to what is provided.
  - "false" = Number will be added to the current city health.


### Empire map: Convert future trade city
+ type = empire_map_convert_future_trade_city
+ target_city = The name of the city that you want to convert to a tradable city.
  - Note: Target city must be defined as a future_trade city.
  - WARNING! If you change the empire map / cities after importing this, then it may not be pointing to the correct route anymore!
  - The name of the city is only checked once at the point of import.
  - If you change the empire map / cities, then re-import the scenario events afterwards.
+ show_message = Should we show the player an 'empire expanded' message? Allowed values: "false" or "true"
  - "false" = No message is shown.
  - "true" = The empire expanded message is shown.


### Favour adjustment
+ type = favor_add
+ amount = The amount to adjust favour by. Allowed values: -100 to 100


### Gladiator revolt
+ type = gladiator_revolt
  - Note: This does not take any additional parameters. It simply starts a gladiator revolt.
  - Does nothing if a gladiator revolt is already underway, or if starting one is not possible.
  - WARNING! If you are using this action, avoid using the editor gladiator revolt option. The two can clash and behave unexpectedly.


### Money
+ type = money_add
+ min = The minimum amount to adjust money by. Allowed values: -10000000 to 10000000. (Negative values remove money.)
+ max = The maximum amount to adjust money by. Allowed values: -10000000 to 10000000. Must be larger than or equal to 'min'.


### Resource produced / available locally
+ type = change_resource_produced
+ resource = What resource to change. Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ produced = Should this resource be available in our city? Allowed values: "false" or "true".
  - "false" = Resource will no longer be available in our city (if it was). Related buildings no longer available.
  - "true" = Resource is now available in our city (if it wasn't). Related buildings can be built.


### Request: Start immediately
+ type = request_immediately_start
  - Note: Can be used to have a request repeat more than once.
  - Note: Will not do anything if that specific request is already active. (i.e. player has received the request but not fulfilled or ignored it yet.)
  - Note: If you use this to start a request earlier than it is setup for, it will no longer run at its predefined time. (This action basically overwrites the request to start immediately, and start over if it has been fulfilled before.)
  - Note: You cannot use this event at the start of the scenario, at least 1 month must pass before it can be used. (Does nothing at the start of the scenario.)
+ request_id = The id of the request to start immediately. Allowed values: 0 to 19.
  - 0 is the first request in the top left of the list in the editor.
  - 1 is the second slot on the left, just below the first request.
  - 9 is the last slot on the left.
  - 10 is the top slot on the right.
  - 19 is the last request in the bottom right of the list in the editor.
  - Using a request_id for a request that doesn't exist (not set up) won't do anything.
  - WARNING! If you add, remove or reorder requests. keep in mind that these Id will then point to a different request. (i.e. they point to the slot, not the specific request.)
  - So make sure to check these ids if you move requests.


### Rome wages
+ type = change_rome_wages
  - Note: Rome's wages can never drop below 1.
  - Note: If you use this, you probably want to not make use of the scenario's random events for adjusting wages, as the they will likely 'fight' one another.
+ min = The minimum amount to adjust Rome's wages by. Allowed values: -10000 to 10000.
+ max = The maximum amount to adjust Rome's wages by. Allowed values: -10000 to 10000. Must be larger than or equal to 'min'.
+ set_to_value = Should we set the wages to the given value, instead of adding it? Allowed values: "false" or "true".
  - "true" = Rome's wages will just be set to what is provided.


### Savings
+ type = savings_add
+ min = The minimum amount to adjust savings by. Allowed values: -10000000 to 10000000. (Negative values remove savings.)
+ max = The maximum amount to adjust savings by. Allowed values: -10000000 to 10000000. Must be larger than or equal to 'min'.


### Send standard message
+ type = send_standard_message
+ text_id = The name of the message to display to the player.
  - Allowed values: A very big list. Refer to: (special_attribute_mappings_standard_message) in code.


### Set Tax Rate
+ type = tax_rate_set
+ amount = The new tax rate to set for the player.
  - Allowed values: Any number from 0 to 100.
  - Note: This does not prevent the player from changing it again afterwards.


### Show custom message
+ type = show_custom_message
+ message_uid = The unique name (uid) of the custom message to display to the player.
  - Allowed values: Any uid of any message from custom messages that you have imported.


### Trade good price
+ type = trade_price_adjust
+ resource = What resource to use. Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ amount = The amount to adjust the trade price (buy and sell). Allowed values: -10000 to 10000.
+ show_message = Should we show the normal 'price changed' message? Allowed values: "false" or "true"


### Trade route amount
+ type = trade_route_amount
+ target_city = The name of the city that you want to adjust the route of.
  - WARNING! If you change the empire map / cities after importing this, then it may not be pointing to the correct route anymore!
  - The name of the city is only checked once at the point of import.
  - If you change the empire map / cities, then re-import the scenario events afterwards.
+ resource = What resource to use. Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ amount = The amount to adjust the trade price (buy and sell). Allowed values: -10000 to 10000.
+ show_message = Should we show the normal 'price changed' message? Allowed values: "false" or "true"
  - "false" = No message is shown.
  - "true" = The standard 'price has changed' message is shown.


### Trade route change opening price
+ type = change_trade_route_open_price
+ target_city = The name of the city that you want to adjust the route of.
  - WARNING! If you change the empire map / cities after importing this, then it may not be pointing to the correct route anymore!
  - The name of the city is only checked once at the point of import.
  - If you change the empire map / cities, then re-import the scenario events afterwards.
+ amount = The amount to adjust the opening price by (or set it to if set_to_value = true). Allowed values: -1000000000 to 1000000000.
  - Note: The final price will be set to 0 instead if it would result in a negative cost.
  - Note: This does not close an already open route.
+ set_to_value = Should we set the city health to the given value, instead of adding it? Allowed values: "false" or "true".
  - "true" = The price will be set to what is given in 'value'.
  - "false" = The 'value' will be added to the opening price of the route.
+ show_message = Should we show the a 'route cost changed' message? Allowed values: "false" or "true"
  - "false" = No message is shown.
  - "true" = A 'route cost changed' message is shown. Displays city name and new opening price.


### Trade: Land trade problems
+ type = trade_problems_land
+ duration = The duration in days of the disruption. Allowed values: 0 to 10000.
  - Remember, in the game there are only 16 'days' in a month.
  - Be careful! Players are used to trade problems lasting only 3 months. (48 days)


### Trade: Sea trade problems
+ type = trade_problems_sea
+ duration = The duration in days of the disruption. Allowed values: 0 to 10000.
  - Remember, in the game there are only 16 'days' in a month.
  - Be careful! Players are used to trade problems lasting only 3 months. (48 days)


### Trade good set price
+ type = trade_price_set
+ resource = What resource to set the price of. Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ amount = The new value to set the good price to. Allowed values: 0 to 100000.
+ set_buy_price = Should we set the buy price to the new value? Allowed values: "false" or "true"
  - "false" = The trade good price will be adjusted so that its sell price is equal to the given amount.
  - "true" = The trade good price will be adjusted so that its buy price is equal to the given amount.
+ show_message = Should we show the normal 'price changed' message? Allowed values: "false" or "true"
  - "false" = No message is shown.
  - "true" = The standard 'price has changed' message is shown.


### Trade: Add new resource to route
+ type = trade_route_add_new_resource
+ target_city = The name of the city that you want to adjust the route of.
  - WARNING! If you change the empire map / cities after importing this, then it may not be pointing to the correct route anymore!
  - The name of the city is only checked once at the point of import.
  - If you change the empire map / cities, then re-import the scenario events afterwards.
+ resource = What resource to set the price of. Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ amount = The yearly trade limit of the resource being added. Allowed values: 0 to 1000000000.
+ add_as_buying = Should the new resource be added to the target route as being bought by that route? Allowed values: "false" or "true"
  - "false" = The route will sell the new resource.
  - "true" = The route will buy the new resource.
+ show_message = Should we show the normal 'increased trade' message? Allowed values: "false" or "true"
  - "false" = No message is shown.
  - "true" = The standard 'increased trade' message is shown.


### Trade: Open route
+ type = trade_route_set_open
+ target_city = The name of the city that you want to adjust the route of.
  - WARNING! If you change the empire map / cities after importing this, then it may not be pointing to the correct route anymore!
  - The name of the city is only checked once at the point of import.
  - If you change the empire map / cities, then re-import the scenario events afterwards.
+ apply_cost = Should the trade route open cost be subtracted from the player's money? Allowed values: "false" or "true"
  - "false" = The route is opened for free.
  - "true" = The money is deducted from the player's money.


### Trade: Set only the buy price for a resource
+ type = trade_set_buy_price_only
+ resource = What resource to set only the buy price of (sell price remains unchanged). Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ amount = The new price. Allowed values: 0 to 1000000000.
  - If new buy price is less than 2, will be set to 2 instead.


### Trade: Set only the sell price for a resource
+ type = trade_set_sell_price_only
+ resource = What resource to set only the sell price of (buy price remains unchanged). Allowed values: (Any resource name) "wheat", "timber", "marble", etc.
+ amount = The new price. Allowed values: 0 to 1000000000.
  - If new sell price is less than 1, will be set to 1 instead.

