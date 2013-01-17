// This file was autogenerated from src/ca/stdlib.ca

namespace circa {

extern "C" {

const char* STDLIB_CA_TEXT = 
    "\n"
    "-- Builtin functions\n"
    "def convert(any value, Type t) -> any\n"
    "    -- Convert the value to the given type, if possible.\n"
    "\n"
    "def assert(bool condition)\n"
    "    -- Raises an error if 'condition' is false.\n"
    "\n"
    "def length(List list) -> int\n"
    "    -- Return the number of items in the given list\n"
    "\n"
    "def from_string(String s) -> any\n"
    "    -- Parse a string representation to a Circa value\n"
    "\n"
    "def to_string_repr(any val) -> String\n"
    "    -- Encode a Circa value as a string representation\n"
    "\n"
    "def has_effects()\n"
    "    -- Declare that a function has effects. Used in native-patched functions.\n"
    "\n"
    "def native_patch(String filename)\n"
    "    -- Declare that this block should be patched by a native module.\n"
    "\n"
    "def changed(any current) -> bool\n"
    "    state any prev = null\n"
    "    result = prev != current\n"
    "    prev = current\n"
    "    return result\n"
    "\n"
    "def first_name(any val) -> Symbol\n"
    "    if type(val) == Symbol\n"
    "        val\n"
    "    elif type(val) == List\n"
    "        if length(val) > 0\n"
    "            first_name(val[0])\n"
    "\n"
    "def dynamic_call(Block block, List inputs) -> List\n"
    "    -- Dynamic function dispatch\n"
    "\n"
    "-- Modules\n"
    "def require(String moduleName)\n"
    "def package(String moduleName)\n"
    "def import_file(String moduleName, String filename)\n"
    "def native_patch_this(String filename)\n"
    "\n"
    "-- Message passing\n"
    "def send(String name, any msg)\n"
    "    -- Pass a message to an actor by name\n"
    "def call_actor(String name, any msg)\n"
    "    -- Invoke an actor immediately\n"
    "def actor_run_message(Actor actor, any msg)\n"
    "    b = actor.block\n"
    "    dynamic_call(b, msg)\n"
    "\n"
    "def test_oracle() -> any\n"
    "    -- For internal testing. This function will output values that are manually\n"
    "    -- inserted with the C++ function oracle_send.\n"
    "\n"
    "def test_spy(any val)\n"
    "    -- For internal testing. This function will save inputs to a global list which\n"
    "    -- can be easily examined from C++ code.\n"
    "\n"
    "type Mutable;\n"
    "def Mutable.set(self, any val)\n"
    "def Mutable.get(self) -> any\n"
    "\n"
    "namespace cppbuild\n"
    "    def build_module(String baseFilename)\n"
    "\n"
    "namespace file\n"
    "    def exists(String filename) -> bool\n"
    "    def version(String filename) -> int\n"
    "    def read_text(String filename) -> String\n"
    "\n"
    "    def file_changed(String filename) -> bool\n"
    "        ver = version(filename)\n"
    "        return changed([filename ver])\n"
    "\n"
    "namespace refactor\n"
    "    def rename(Term r, String s)\n"
    "    def change_function(Term r, Callable func)\n"
    "\n"
    "namespace reflect\n"
    "    def this_block() -> Block\n"
    "    def kernel() -> Block\n"
    "\n"
    "namespace sys\n"
    "    def arg(int index) -> String\n"
    "    def dll_patch()\n"
    "    def module_search_paths() -> List\n"
    "    def perf_stats_reset()\n"
    "    def perf_stats_dump() -> List\n"
    "\n"
    "-- Builtin types\n"
    "type Actor { String name, String moduleName, List incomingQueue, any stateVal }\n"
    "type FileSignature { String filename, int time_modified }\n"
    "type Callable;\n"
    "type Color { number r, number g, number b, number a }\n"
    "type Point { number x, number y }\n"
    "type Point_i { int x, int y }\n"
    "type Rect { number x1, number y1, number x2, number y2 }\n"
    "type Rect_i { int x1 int y1 int x2 int y2 }\n"
    "\n"
    "-- Metaprogramming on Block\n"
    "def block_ref(any block :ignore_error) -> Block\n"
    "    -- Obtain a Block ref from an expression.\n"
    "def Block.call(self, any inputs :multiple) -> List\n"
    "    -- Invoke this Block with the given inputs.\n"
    "def Block.dump(self)\n"
    "    -- Dump this block's raw contents to stdout.\n"
    "def Block.file_signature(self) -> any\n"
    "    -- Return a 'file signature', which contains the filename and last-modified time.\n"
    "def Block.find_term(self, String name) -> Term\n"
    "    -- Find a term inside this Block with the given name.\n"
    "def Block.format_source(self) -> List\n"
    "    -- Return the block's contents as formatted source.\n"
    "def Block.format_function_heading(self) -> List\n"
    "    -- Return a formatted function header for this block.\n"
    "def Block.functions(self) -> List\n"
    "    -- Return a list of functions that occur inside this block.\n"
    "def Block.get_term(self, int index) -> Term\n"
    "    -- Fetch a term by index.\n"
    "def Block.get_static_errors(self) -> List\n"
    "    -- Return a raw list of static errors inside this block.\n"
    "def Block.get_static_errors_formatted(self) -> List\n"
    "    -- Return a formatted list of static errors inside this block.\n"
    "def Block.has_static_error(self) -> bool\n"
    "    -- Return whether this block has any static errors.\n"
    "def Block.input(self, int index) -> Term\n"
    "    -- Fetch an input placeholder term by index.\n"
    "def Block.is_null(self) -> bool\n"
    "    -- Return whether this is a null Block reference.\n"
    "def Block.inputs(self) -> List\n"
    "    -- Return a list of input placeholder terms.\n"
    "def Block.link(self, Block lib)\n"
    "    -- Iterate through this block, looking for terms that have missing references.\n"
    "    -- For each missing reference, if a term with the expected name is found in 'lib',\n"
    "    -- then modify the term to use the named term in 'lib'.\n"
    "def Block.list_configs(self) -> List\n"
    "    -- Return a list of Terms that look like configs (named literal values).\n"
    "def Block.output(self, int index) -> Term\n"
    "    -- Return an output placeholder term by index.\n"
    "def Block.outputs(self) -> List\n"
    "    -- Return a list of all output placeholders.\n"
    "def Block.owner(self) -> Term\n"
    "    -- Return the Term that owns this Block (may be null).\n"
    "def Block.property(self, String name) -> any\n"
    "def Block.properties(self) -> Map\n"
    "def Block.statements(self) -> List\n"
    "    -- Return a list of Terms that are statements.\n"
    "def Block.terms(self) -> List\n"
    "    -- Return a list of this block's terms.\n"
    "def Block.version(self) -> int\n"
    "    -- Return the 'version', which is a monotonically increasing number that is\n"
    "    -- incremented whenever the block is modified. Useful for change tracking.\n"
    "def Block.walk_terms(self) -> List\n"
    "    -- Return a list of this block's terms, and all nested terms.\n"
    "\n"
    "type Closure {\n"
    "    Block block\n"
    "    List bindings\n"
    "}\n"
    "def Closure.call(self, any inputs :multiple) -> any\n"
    "\n"
    "def closure_block() -> Closure\n"
    "def unbound_input(any _) -> any\n"
    "    -- Internally used function, used inside closure blocks to mark an unbound input.\n"
    "\n"
    "def Dict.count(self) -> int\n"
    "    -- Return the number of elements\n"
    "def Dict.set(self, String key, any value) -> Dict\n"
    "    -- Assign a key-value pair.\n"
    "def Dict.get(self, String key) -> any\n"
    "    -- Retrieve the value for a given key.\n"
    "\n"
    "type Frame;\n"
    "def Frame.block(self) -> Block\n"
    "    -- Return the Block associated with this Frame.\n"
    "def Frame.register(self, int index) -> any\n"
    "    -- Fetch the value in the given register index.\n"
    "def Frame.registers(self) -> List\n"
    "    -- Fetch a list of all register values.\n"
    "def Frame.pc(self) -> int\n"
    "    -- Fetch the current program counter (the interpreter's current position).\n"
    "def Frame.parentPc(self) -> int\n"
    "def Frame.pc_term(self) -> Term\n"
    "    -- Fetch the term associated with the current program counter.\n"
    "def Frame.create_expansion(self, Term term) -> Frame\n"
    "    -- Create an expansion frame using the given Term.\n"
    "    -- Errors if the Term is not part of this frame's block.\n"
    "def Frame.inputs(self) -> List\n"
    "    -- Returns a list of input register values\n"
    "def Frame.input(self, int nth) -> any\n"
    "    -- Returns the nth input register value\n"
    "def Frame.outputs(self) -> List\n"
    "    -- Returns a list of output register values\n"
    "def Frame.output(self, int nth) -> any\n"
    "    -- Returns the nth output register value\n"
    "\n"
    "def Function.block(self) -> Block\n"
    "\n"
    "type Interpreter;\n"
    "def make_interpreter() -> Interpreter\n"
    "    -- Create a new interpreter\n"
    "def Interpreter.push_frame(self, Block b, List inputs)\n"
    "    -- Push a new frame, using the given block and input list.\n"
    "def Interpreter.pop_frame(self)\n"
    "    -- Pop the topmost frame.\n"
    "def Interpreter.set_state_input(self, any val)\n"
    "    -- Assign a value to the :state input of the topmost frame (if any)\n"
    "def Interpreter.get_state_output(self) -> any\n"
    "def Interpreter.reset(self)\n"
    "    -- Clear this interpreter's state, including stack frames.\n"
    "def Interpreter.run(self)\n"
    "    -- Run the interpreter until either the topmost frame is finished, or an error\n"
    "    -- is raised.\n"
    "def Interpreter.run_steps(self, int steps)\n"
    "    -- Run the interpreter for a specific number of steps. If an error occurs then\n"
    "    -- evaluation will be stopped.\n"
    "def Interpreter.frame(self, int depth) -> Frame\n"
    "    -- Return the Frame with the given depth. The top of the stack has depth 0.\n"
    "def Interpreter.top_frame(self) -> Frame\n"
    "    -- Return the topmost Frame.\n"
    "    self.frame(0)\n"
    "def Interpreter.frames(self) -> List\n"
    "    -- Return the list of Frames.\n"
    "def Interpreter.output(self, int index) -> any\n"
    "    -- Fetch the value in the nth output register.\n"
    "def Interpreter.errored(self) -> bool\n"
    "    -- Returns true if the interpreter has stopped to due error.\n"
    "def Interpreter.error_message(self) -> String\n"
    "    -- If there is a recorded error, returns a human-readable description string.\n"
    "def Interpreter.toString(self) -> String\n"
    "\n"
    "def empty_list(any initialValue, int size) -> List\n"
    "def List.append(self, any item) -> List\n"
    "    -- Append an item to the end of this list.\n"
    "def List.concat(self, List rightSide) -> List\n"
    "    -- Concatenate two lists.\n"
    "def List.resize(self, int length) -> List\n"
    "    -- Resize the list to have the given length. If the length is increased, then\n"
    "    -- null values are appended on the right; if the length is decreased then\n"
    "    -- existing values on the right are dropped.\n"
    "def List.count(self) -> int\n"
    "    -- Return the number of elements in this list.\n"
    "\n"
    "def List.insert(self, int index, any val) -> List\n"
    "    -- Insert an element at the given index. If necessary, existing elements will\n"
    "    -- be shifted to the right to make room.\n"
    "    --\n"
    "    -- Example:\n"
    "    --   a = [1 2 3]\n"
    "    --   a.insert(1, 'X')\n"
    "    --   -- a now equals [1 'X' 2 3]\n"
    "    \n"
    "def List.length(self) -> int\n"
    "    -- Return the number of elements in this list.\n"
    "\n"
    "def List.join(self, String joiner) -> String\n"
    "    -- Return a string constructed by converting every element to a string, and\n"
    "    -- concatenating those strings, each separated by 'joiner'.\n"
    "    --\n"
    "    -- Example:\n"
    "    --  [1 2 3].join(', ')\n"
    "    -- \n"
    "    -- Outputs: '1, 2, 3'\n"
    "\n"
    "def List.slice(self, int start, int fin) -> List\n"
    "    -- Return a list constructed from the elements starting from index 'start', and\n"
    "    -- ending immediately before index 'fin'.\n"
    "    --\n"
    "    -- If either 'start' or 'fin' is negative, it's interpreted as an offset from the\n"
    "    -- end of the list.\n"
    "    --\n"
    "    -- Examples:\n"
    "    --  [1 2 3 4].slice(1 3)\n"
    "    --  Outputs: [2 3]\n"
    "    -- \n"
    "    --  [1 2 3 4].slice(0 -1)\n"
    "    --  Outputs: [1 2 3]\n"
    "\n"
    "def List.get(self, int index) -> any\n"
    "    -- Get an element by index.\n"
    "\n"
    "def List.get_optional(self, int index, any fallback) -> any\n"
    "    -- Get an element by index if it exists, otherwise return 'fallback'.\n"
    "    if index < 0 or index >= self.length\n"
    "        fallback\n"
    "    else\n"
    "        self.get(index)\n"
    "\n"
    "def List.set(self, int index, any val) -> List\n"
    "    -- Set an element by index.\n"
    "def List.empty(self) -> bool\n"
    "    -- Returns whether the list is empty.\n"
    "    self.length == 0\n"
    "def List.first(self) -> any\n"
    "    -- Returns the first element\n"
    "    self.get(0)\n"
    "def List.last(self) -> any\n"
    "    -- Returns the last element\n"
    "    self.get(self.length - 1)\n"
    "\n"
    "-- def List.pop(self) -> (List, any)\n"
    "-- TODO: List.pop, needs proper support for multiple outputs\n"
    "\n"
    "def List.filter(self, Closure func) -> List\n"
    "    result = []\n"
    "    for i in self\n"
    "        if func.call(i)\n"
    "            result.append(i)\n"
    "    result\n"
    "\n"
    "def List.map(self, Closure func) -> List\n"
    "    for i in self\n"
    "        func.call(i)\n"
    "\n"
    "def List.mapItem(self, int index, Closure func) -> List\n"
    "    self.set(index, func.call(self[index]))\n"
    "\n"
    "def Map.contains(self, any key) -> bool\n"
    "    -- Returns true if the map contains the given key.\n"
    "def Map.remove(self, any key) -> Map\n"
    "    -- Removes the given key from the map.\n"
    "def Map.get(self, any key) -> any\n"
    "    -- Gets the key associated with this value.\n"
    "def Map.set(self, any key, any value) -> Map\n"
    "    -- Sets the value associated with this key.\n"
    "def Map.insertPairs(self, List pairs) -> Map\n"
    "    -- Set multiple key-value pairs.\n"
    "\n"
    "-- Overloaded functions\n"
    "def is_overloaded_func(Block block) -> bool\n"
    "    -- Return true if this block is an overloaded function\n"
    "\n"
    "def overload_get_contents(Block block) -> List\n"
    "    -- Get a list of all the functions that this overloaded function uses.\n"
    "\n"
    "def String.append(self, String right) -> String\n"
    "    -- Append a string to the right side.\n"
    "    concat(self, right)\n"
    "    \n"
    "def String.char_at(self, int index) -> String\n"
    "\n"
    "def String.ends_with(self, String suffix) -> bool\n"
    "    -- Return true if this string ends with the given substring.\n"
    "def String.length(self) -> int\n"
    "    -- Returns the string length.\n"
    "def String.slice(self, int start, int fin) -> String\n"
    "def String.starts_with(self, String prefix) -> bool\n"
    "    -- Returns true if the string starts with the given prefix.\n"
    "def String.split(self, String sep) -> List\n"
    "def String.substr(self, int start, int fin) -> String\n"
    "def String.to_camel_case(self) -> String\n"
    "    -- Deprecated function\n"
    "def String.to_lower(self) -> String\n"
    "def String.to_upper(self) -> String\n"
    "\n"
    "def String.characters(self) -> List\n"
    "    out = for i in 0..(self.length)\n"
    "        self.char_at(i)\n"
    "    return out\n"
    "\n"
    "def Type.name(self) -> String\n"
    "    -- Returns this Term's local name. May be empty.\n"
    "def Type.property(self, String name) -> any\n"
    "def Type.declaringTerm(self) -> Term\n"
    "\n"
    "def type(any val) -> Type\n"
    "    -- Returns the runtime type of a value\n"
    "def static_type(any term :meta) -> Type\n"
    "    -- Returns the static type of a value\n"
    "\n"
    "-- Metaprogramming on Term\n"
    "def term_ref(any term :ignore_error) -> Term\n"
    "def Term.assign(self, any val)\n"
    "def Term.asint(self) -> int\n"
    "def Term.asfloat(self) -> number\n"
    "def Term.parent(self) -> Block\n"
    "def Term.contents(self) -> Block\n"
    "    -- Fetch the nested Block contents of this Term. The format and the meaning\n"
    "    -- of these contents is dictated by the term's function.\n"
    "def Term.is_null(self) -> bool\n"
    "    -- Returns whether this is a null Term reference.\n"
    "def Term.name(self) -> String\n"
    "    -- Return this term's local name. May be blank.\n"
    "def Term.to_string(self) -> String\n"
    "def Term.to_source_string(self) -> String\n"
    "def Term.format_source(self) -> List\n"
    "    -- Return a list of formatted source phrases for this Term\n"
    "def Term.format_source_normal(self) -> List\n"
    "    -- Return a list of formatted source phrases for this Term. This formatting is 'normal',\n"
    "    -- meaning that it doesn't include the source for inputs (inputs are referred to by name\n"
    "    -- or by id), and special syntax is not used.\n"
    "def Term.function(self) -> Block\n"
    "    -- Fetch this term's function.\n"
    "def Term.get_type(self) -> Type\n"
    "    -- Fetch the declared type of this term.\n"
    "def Term.value(self) -> any\n"
    "    -- For a value term, this fetches the actual value.\n"
    "def Term.tweak(self, number steps)\n"
    "def Term.input(self, int index) -> Term\n"
    "    -- Fetch an input term by index.\n"
    "def Term.inputs(self) -> List\n"
    "    -- Return a list of input terms.\n"
    "def Term.num_inputs(self) -> int\n"
    "    -- Return the number of inputs.\n"
    "def Term.source_location(self) -> Rect_i\n"
    "    -- Return a Rect_i that describes where this term occurs in its source file.\n"
    "def Term.location_string(self) -> String\n"
    "def Term.global_id(self) -> int\n"
    "    -- Fetch this term's global identifier.\n"
    "def Term.properties(self) -> Map\n"
    "def Term.property(self, String name) -> any\n"
    "    -- Fetch a term property by name.\n"
    "def Term.is_value(self) -> bool\n"
    "    self.function == block_ref(value)\n"
    "def Term.is_function(self) -> bool\n"
    "    self.is_value and is_function(self.value)\n"
    "def Term.is_type(self) -> bool\n"
    "    self.is_value and is_type(self.value)\n"
    "def Term.is_input_placeholder(self) -> bool\n"
    "    self.function == block_ref(input_placeholder)\n"
    "def Term.is_comment(self) -> bool\n"
    "    self.function == block_ref(comment)\n"
    "\n"
    "-- workaround, these functions currently need to be declared after Term:\n"
    "def Block.name(self) -> String\n"
    "    self.owner.name\n"
    "\n"
    "def Block.is_method(self) -> bool\n"
    "    self.owner.property(\"syntax:methodDecl\") == true\n"
    "\n"
    "def Block.get_top_comments(self) -> List\n"
    "    out = []\n"
    "    for Term t in self.terms\n"
    "        if t.is_input_placeholder\n"
    "            continue\n"
    "        elif t.is_comment\n"
    "            str = t.property('comment')\n"
    "            if str == \"\"\n"
    "                break\n"
    "\n"
    "            out.append(t.property('comment'))\n"
    "        else\n"
    "            break\n"
    "\n"
    "    return out\n"
    "\n"
    "def Point.magnitude(self) -> number\n"
    "    -- Return the magnitude of this point, or the distance to [0 0].\n"
    "    sqrt(sqr(self.x) + sqr(self.y))\n"
    "\n"
    "def Point.norm(self) -> Point\n"
    "    -- Returns v normalized to be along the unit circle.\n"
    "    m = magnitude(self)\n"
    "    [self.x / m, self.y / m]\n"
    "\n"
    "def Point.perpendicular(self) -> Point\n"
    "    -- Returns a vector that is perpendicular to self, rotated clockwise.\n"
    "    [self.y -self.x]\n"
    "\n"
    "def Point.to_rect_center(self, Point size) -> Rect\n"
    "    -- Returns a Rect with this point as the center, and the given size.\n"
    "    size_half = (size * 0.5) -> Point\n"
    "    [self.x - size_half.x,\n"
    "        self.y - size_half.y, self.x + size_half.x, self.y + size_half.y]\n"
    "\n"
    "def Point.to_rect_topleft(self, Point size) -> Rect\n"
    "    -- Returns a Rect with this point as the top-left, and the given size.\n"
    "    [self.x, self.y, self.x + size.x, self.y + size.y]\n"
    "\n"
    "def Point.delta(self) -> Point\n"
    "    -- Stateful function, returns the point difference since the last frame.\n"
    "    state Point prev = self\n"
    "    result = [self.x - prev.x, self.y - prev.y]\n"
    "    prev = self\n"
    "    result\n"
    "\n"
    "def Point.distance(self, Point b) -> number\n"
    "    -- Returns the distance between self and b.\n"
    "    sqrt(sqr(self.x - b.x) + sqr(self.y - b.y))\n"
    "\n"
    "def Point.rotate(self, number rotation) -> Point\n"
    "    -- Return the point rotated around [0 0] by the given angle.\n"
    "    [self.x*cos(rotation) - self.y*sin(rotation), self.x*sin(rotation)+self.y*cos(rotation)]\n"
    "\n"
    "def Rect.width(self) -> number\n"
    "    self.x2 - self.x1\n"
    "def Rect.height(self) -> number\n"
    "    self.y2 - self.y1\n"
    "def Rect.topleft(self) -> Point\n"
    "    -- Returns the top-left coordinates as a Point.\n"
    "    [self.x1 self.y1]\n"
    "def Rect.topright(self) -> Point\n"
    "    -- Returns the top-rigth coordinates as a Point.\n"
    "    [self.x2 self.y1]\n"
    "def Rect.bottomleft(self) -> Point\n"
    "    -- Returns the bottom-left coordinates as a Point.\n"
    "    [self.x1 self.y2]\n"
    "def Rect.bottomright(self) -> Point\n"
    "    -- Returns the bottom-right coordinates as a Point.\n"
    "    [self.x2 self.y2]\n"
    "def Rect.center(self) -> Point\n"
    "    -- Returns the rectangle's center as a Point\n"
    "    [(self.x1 + self.x2) / 2, (self.y1 + self.y2) / 2]\n"
    "\n"
    "def Rect.intersects(self, Rect b) -> bool\n"
    "    -- Return whether the two rectangles intersect.\n"
    "    intersects_on_x = (self.x2 > b.x1) and (b.x2 > self.x1)\n"
    "    intersects_on_y = (self.y2 > b.y1) and (b.y2 > self.y1)\n"
    "    return intersects_on_x and intersects_on_y\n"
    "\n"
    "def Rect.translate(self, Point p) -> Rect\n"
    "    [self.x1 + p.x, self.y1 + p.y, self.x2 + p.x, self.y2 + p.y]\n"
    "\n"
    "def Rect.contains(self, Point p) -> bool\n"
    "    p.x >= self.x1 and p.y >= self.y1 and p.x < self.x2 and p.y < self.y2\n"
    "\n"
    "def sum(List numbers) -> number\n"
    "    result = 0\n"
    "    for i in numbers\n"
    "        result += i\n"
    "    result\n"
    "    \n"
    "\n"
    "def zip(List left, List right) -> List\n"
    "    for l in left\n"
    "        [l, right.get_optional(index(), null)]\n"
    "\n"
    "-- Stateful functions\n"
    "\n"
    "def toggle(bool tog) -> bool\n"
    "    -- Stateful function, returns a boolean status. Every frame the function is called\n"
    "    -- with (true), the result flips. Starts out false.\n"
    "\n"
    "    state bool s = false\n"
    "    if tog\n"
    "        s = not(s)\n"
    "    return s\n"
    "\n"
    "def approach(number target, number maximum_change) -> number\n"
    "    -- Stateful function, returns a result which approaches 'target'. Each time the\n"
    "    -- function is called, the result will change at most by 'maximum_change'.\n"
    "    state current = target\n"
    "    if target > current\n"
    "        current += min(maximum_change, target - current)\n"
    "    elif target < current\n"
    "        current -= min(maximum_change, current - target)\n"
    "    return current\n"
    "\n"
    "def once() -> bool\n"
    "    -- Stateful function, returns true the first time it's called, and false thereafter.\n"
    "    state bool s = true\n"
    "    result = s\n"
    "    s = false\n"
    "    return result\n"
    "\n"
    "def cycle(int max) -> int\n"
    "    -- Stateful function, cycles though the integers from 0 to (max - 1). When the maximum\n"
    "    -- is reached, the function will start again at 0.\n"
    "\n"
    "    state int counter = 0\n"
    "    if counter >= max\n"
    "        counter = 0\n"
    "    result = counter\n"
    "    counter += 1\n"
    "    return result\n"
    "\n"
    "def cycle_elements(List list) -> any\n"
    "    -- Stateful function, cycles through each item of 'list' one at a time.\n"
    "    return list[cycle(length(list))]\n"
    "\n"
    "def random_element(List list) -> any\n"
    "    -- Return a random element from the given list, with equal probability per element.\n"
    "    return list[rand_i(length(list))]\n"
    "\n"
    "def seed() -> number\n"
    "    -- Stateful function, returns a random number 0..1 which doesn't change after initialization.\n"
    "    state number s = rand()\n"
    "    return s\n"
    "\n"
    "-- Math utility functions\n"
    "def polar(number angle) -> Point\n"
    "    -- Return a point on the unit circle with the given angle.\n"
    "    return [sin(angle) -cos(angle)]\n"
    "\n"
    "def magnitude(Point point) -> number\n"
    "    -- Returns the magnitude, or the distance from [0,0] to this point.\n"
    "    return sqrt(sqr(point.x) + sqr(point.y))\n"
    "\n"
    "def point_distance(Point a, Point b) -> number\n"
    "    -- Returns the distance between a and b.\n"
    "    sqrt(sqr(a.x - b.x) + sqr(a.y - b.y))\n"
    "\n"
    "def rotate_point(Point p, number rotation) -> Point\n"
    "    -- Rotate p around [0,0] by the given angle.\n"
    "    return [p.x*cos(rotation) - p.y*sin(rotation), p.x*sin(rotation)+p.y*cos(rotation)]\n"
    "\n"
    "def random_norm_vector() -> Point\n"
    "    -- Return a random normalized vector.\n"
    "    angle = rand() * 360\n"
    "    return [cos(angle) sin(angle)]\n"
    "\n"
    "def rect(number x1, number y1, number x2, number y2) -> Rect\n"
    "    return [x1 y1 x2 y2]\n"
    "\n"
    "def to_rect(Point topLeft, Point size) -> Rect\n"
    "    -- Return a Rect with the given top-left coordinate and size.\n"
    "    return [topLeft.x, topLeft.y, topLeft.x + size.x, topLeft.y + size.y]\n"
    "\n"
    "def expand_rect(Point center, Point size) -> Rect\n"
    "    -- Return a Rect with the given center coordinate and size.\n"
    "    width = abs(size.x)\n"
    "    height = abs(size.y)\n"
    "    return [center.x - width/2, center.y - height/2, center.x + width / 2, center.y + height/2]\n"
    "\n"
    "def grow_rect(Rect r, number amount) -> Rect\n"
    "    -- Return a Rect based on r, where each side is moved away from the center by 'amount'.\n"
    "    return Rect([r.x1 - amount, r.y1 - amount, r.x2 + amount, r.y2 + amount])\n"
    "\n"
    "def approach_rect(Rect target, number maxChange) -> Rect\n"
    "    -- Stateful function, returns a Rect that gradually approaches the target Rect.\n"
    "    -- Each coordinate will change by at most 'maxChange' per call.\n"
    "    return Rect([approach(target.x1,maxChange), approach(target.y1,maxChange),\n"
    "        approach(target.x2,maxChange),approach(target.y2,maxChange)])\n"
    "\n"
    "def interp_rect(Rect a, Rect b, number ratio) -> Rect\n"
    "    -- Linearly interpolate between two rectangles.\n"
    "    min(@ratio, 1.0)\n"
    "    result = for i in 0..4 { a[i] + (b[i] - a[i]) * ratio }\n"
    "    return Rect(result)\n"
    "\n"
    "def bezier3(number t, List points) -> Point\n"
    "    -- Quadratic bezier curve, with 3 control points. t must be in the range of [0, 1].\n"
    "    p0 = points[0] -> Point\n"
    "    p1 = points[1] -> Point\n"
    "    p2 = points[2] -> Point\n"
    "    return p0 * sqr(1 - t) + p1 * (2 * t * (1 - t)) + p2 * sqr(t)\n"
    "\n"
    "def bezier4(number t, List points) -> Point\n"
    "    -- Cubic bezier curve, with 4 control points. t must be in the range of [0, 1].\n"
    "    p0 = points[0] -> Point\n"
    "    p1 = points[1] -> Point\n"
    "    p2 = points[2] -> Point\n"
    "    p3 = points[3] -> Point\n"
    "    return p0 * cube(1 - t) + p1 * (3 * t * sqr(1 - t)) + p2 * sqr(t) * (3 * (1 - t)) + p3 * cube(t)\n"
    "\n"
    "def smooth_in_out(number t, number smooth_in, number smooth_out) -> number\n"
    "    return bezier4(t, [[0 0] [smooth_in, 0] [1 - smooth_out, 1] [1 1]]).y\n"
    "\n"
    "def clamp(number x, number minVal, number maxVal) -> number\n"
    "    -- Return a clamped value. If x is less than 'minVal' then return 'minVal', if it's\n"
    "    -- greater than 'maxVal' then return 'maxVal', otherwise return the original 'x'.\n"
    "    return min(max(x, minVal), maxVal)\n"
    "\n"
    "def smoothstep(number x, number edge0, number edge1) -> number\n"
    "    -- Smooth interpolation func, see en.wikipedia.org/wiki/Smoothstep\n"
    "    x = clamp((x - edge0)/(edge1-edge0), 0, 1)\n"
    "    return x*x*(3 - 2*x)\n"
    "\n"
    "def smootherstep(number x, number edge0, number edge1) -> number\n"
    "    -- Alternative to smoothstep by Ken Perlin, see en.wikipedia.org/wiki/Smoothstep\n"
    "    x = clamp((x - edge0)/(edge1-edge0), 0, 1)\n"
    "    return x*x*x*(x*(x*6 - 15) + 10)\n"
    "\n"
    "def random_color() -> Color\n"
    "    -- Return a color with random RGB components, and full alpha.\n"
    "    return [rand() rand() rand() 1.0]\n"
    "\n"
    "def blend_color(Color a, Color b, number ratio) -> Color\n"
    "    -- Return a color that is blended between a and b. If ratio is 0.0 we'll\n"
    "    -- return 'a', if it's 1.0 we'll return 'b'.\n"
    "    \n"
    "    {-\n"
    "    def unit_tests()\n"
    "        blend_color(#abc #123 0.0) == #abc\n"
    "        blend_color(#abc #123 1.0) == #123\n"
    "        blend_color(#abc #123 2.0) == #123\n"
    "        blend_color(#abc #123 -1.0) == #abc\n"
    "        blend_color(#fff #fff 0.5) == #fff\n"
    "    -}\n"
    "\n"
    "    max(@ratio, 0.0)\n"
    "    min(@ratio, 1.0)\n"
    "\n"
    "    if ratio == 0.0\n"
    "        return a\n"
    "    elif ratio == 1.0\n"
    "        return b\n"
    "    else\n"
    "        result = a + (b - a) * ratio\n"
    "\n"
    "        -- hack to preserve alpha:\n"
    "        result[3] = 1.0\n"
    "        return result\n"
    "\n"
    "def hsv_to_rgb(List hsv) -> Color\n"
    "    -- Convert HSV components to an RGB Color\n"
    "\n"
    "    h = hsv[0] -> number\n"
    "    s = hsv[1] -> number\n"
    "    v = hsv[2] -> number\n"
    "\n"
    "    if s == 0\n"
    "        -- grey\n"
    "        return [v v v 1]\n"
    "\n"
    "    h *= 6\n"
    "    i = floor(h)\n"
    "    f = h - i\n"
    "    p = v * (1 - s)\n"
    "    q = v * (1 - s * f)\n"
    "    t = v * (1 - s * (1 - f))\n"
    "\n"
    "    if i == 0\n"
    "        return [v t p 1]\n"
    "    elif i == 1\n"
    "        return [q v p 1]\n"
    "    elif i == 2\n"
    "        return [p v t 1]\n"
    "    elif i == 3\n"
    "        return [p q v 1]\n"
    "    elif i == 4\n"
    "        return [t p v 1]\n"
    "    elif i == 5\n"
    "        return [v p q 1]\n"
    "\n"
    "    return [0 0 0 0]\n"
    "\n"
    "def Color.lighten(self, number factor) -> Color\n"
    "    [ min(self.r + factor, 1.0)\n"
    "      min(self.g + factor, 1.0)\n"
    "      min(self.b + factor, 1.0)\n"
    "      self.a]\n"
    "        \n"
    "def Color.darken(self, number factor) -> Color\n"
    "    [ max(self.r - factor, 0.0)\n"
    "      max(self.g - factor, 0.0)\n"
    "      max(self.b - factor, 0.0)\n"
    "      self.a]\n"
    "\n"
    "def delta(number val) -> number\n"
    "    -- Stateful function, return the difference between this value and the value from\n"
    "    -- the previous call.\n"
    "    state number prev = val\n"
    "    result = val - prev\n"
    "    prev = val\n"
    "    return result\n";

} // extern "C"

} // namespace circa

