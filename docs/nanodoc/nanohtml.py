
"""

Json document is layed out like this:

headers
  title (string)
[packages]
  name (string)
  [contents]
    name
    function (true or false)
    return_type
    declaration
    [containsOverloads]
    some other stuff

"""

# Parse command-line options
from optparse import OptionParser
options = OptionParser()
options.add_option('--doc', dest = 'doc_filename')
options.add_option('--output', dest = 'output_filename')
(cl_options, cl_args) = options.parse_args()

# Load the entire document
def load_json_doc(filename):
    f = open(filename, 'r')
    file_contents = f.read()
    f.close()

    import json
    doc = json.loads(file_contents)
    return doc

Doc = load_json_doc(cl_options.doc_filename)

# Hide certain functions
FunctionsToHide = set(["annotate_type", "add_feedback", "additional_output",
    "assign", "branch", "sin_feedback", "comment", "copy",
    "cos_feedback", "do_once", "feedback",
    "finish_minor_branch",
    "for", "get_field_by_name","join",
    "get_index", "if_feedback", "mult_feedback", "namespace", "eval_script",
    "get_namespace_field","instance","run_single_statement",
    "set_field", "set_index", "stateful_value", "if_block", "if",
    "input_placeholder", "cond_feedback", "swap", "term_to_source",
    "return",
    "subroutine_output","preserve_state_result","lambda",
    "one_time_assign", "unique_id", "unknown_field", "unknown_function",
    "unknown_identifier", "unsafe_assign", "unrecognized_expr", "unknown_type",
    "vectorize_vs", "vectorize_vv", "value",
    "patch_with_dll"])

def hide_stuff(doc):
    for package in doc['packages']:
        for item in list(package['contents']):
            if item['name'] in FunctionsToHide:
                package['contents'].remove(item)

hide_stuff(Doc)

# Reorganize overloads

def reorganize_document_for_overloads(doc):
    for package in doc['packages']:

        # Make a list of every name that shows up as an overload
        all_overload_names = set()
        for item in package['contents']:
            if 'containsOverloads' in item:
                for name in item['containsOverloads']:
                    all_overload_names.add(name)

        # Remove these items from the regular list
        all_overloads = {} # keyed by name
        for item in list(package['contents']):
            if item['name'] in all_overload_names:
                all_overloads[item['name']] = item
                package['contents'].remove(item)

        # Attach the overloads to the containing functions
        for item in package['contents']:
            if 'containsOverloads' in item:
                item['overloads'] = [all_overloads[name] for name in item['containsOverloads']]

reorganize_document_for_overloads(Doc)

# There's too much stuff in 'builtins', split off some of it into some new modules

MathFunctions = ['abs','add','arccos','arcsin','arctan','average','ceil','cos',
        'decrement',
        'div','div_i', 'floor','increment',
        'length','log','magnitude','max','min','mod','mult',
        'neg','norm','perpendicular','point_distance','polar','pow',
        'range','remainder','round','rotate_point','sin','sqr','sqrt','sub','tan']

DebuggingFunctions = ['dump_parse','dump_scope_state','test_oracle','test_spy']

LogicalFunctions = ['and','any_true','not','or']

FileIOFunctions = ['file_changed','load_script','read_text_file','write_text_file']

ComparisonFunctions = ['equals', 'greater_than', 'greater_than_eq',
        'less_than', 'less_than_eq', 'not_equals']

MetaprogrammingFunctions = ['branch_ref','format_source_for_graph','get_statement_count',
        'lookup_branch_ref','ref']

ColorFunctions = ['blend_color', 'hsv_to_rgb','random_color']

def move_some_builtins_to_new_modules(doc):
    newModules = []

    def declareModule(name, includesNames):
        module = {}
        module['name'] = name
        module['contents'] = []
        module['includesNames'] = includesNames
        newModules.append(module)

    declareModule("&nbsp;&nbsp;color", ColorFunctions)
    declareModule("&nbsp;&nbsp;comparison", ComparisonFunctions)
    declareModule("&nbsp;&nbsp;file i/o", FileIOFunctions)
    declareModule("&nbsp;&nbsp;internal debugging", DebuggingFunctions)
    declareModule("&nbsp;&nbsp;logical", LogicalFunctions)
    declareModule("&nbsp;&nbsp;math", MathFunctions)
    declareModule("&nbsp;&nbsp;metaprogramming", MetaprogrammingFunctions)

    builtinContents = doc['packages'][0]['contents']

    for item in list(builtinContents):
        for newModule in newModules:
            if item['name'] in newModule['includesNames']:
                builtinContents.remove(item)
                newModule['contents'].append(item)

    index = 1
    for module in newModules:
        doc['packages'].insert(index, module)
        index += 1

move_some_builtins_to_new_modules(Doc)

def to_ident(name):
    return name.strip(' ()').lower()

def packageListHTML(packages):
    html = "<div id=\"packages\">"
    html += "<div id=\"packagestitle\">Packages</div>"
    html += "<div id=\"nanobutton\">"
    html += "<ul>"
    
    # Leftmost column: one item for each package
    for package in packages:
        name = package['name']
        html += ("<li><a href=\"javascript:showPackageContents('%s_contents')\">%s</a></li>"
                % (to_ident(name), name))

    html += "</ul>"
    html += "</div>"
    html += "</div>"
    html += "\n"
    return html

def itemListHTML(items):
    html = "<ul>"

    for item in items:
        item_name = item['name']

        html += ('<li><a href="javascript:showSubContents(\'%s_sub\')">%s</a></li>\n'
                % (to_ident(item_name), item_name))

    html += "</ul>"
    return html

def divideListIntoTwo(l):
    left_list = []
    right_list = []
    left = True
    for i in l:
        if left: left_list.append(i)
        else: right_list.append(i)
        left = not left
    return (left_list, right_list)

def detailHTML(item):
    name = item['name']

    html = '<div style="display:none" class="moduledetail" id="%s_sub">' % to_ident(name)
    html += '<div id="moduledetailtitle">%s</div>' % name

    html += '<div id="detailcontent">\n'

    def itemBody(item):
        html = '<span class="detail_declaration">%s</span>\n' % item['declaration']
        html += '<p>'
        html += '<span class="detail_comments">%s</span>\n' % item['comments']
        return html

    html += itemBody(item)

    if 'overloads' in item:
        html += '<p>'
        html += '<br>'
        html += 'Contains the following overloads:'
        for overload in item['overloads']:
            html += '<p>'
            html += itemBody(overload)

    html += "</div>"
    html += "</div>"

    return html

def makeHTML(doc):

    def all_module_contents():
        for module in doc['packages']:
            for item in module['contents']:
                yield item

    def all_functions():
        for item in all_module_contents():
            if 'function' in item:
                yield item

    all_packages = [package for package in doc['packages']]

    # remove hidden functions

    html = ""
    
    html += '<html>\n'
    html += '\t<head>\n'
    html += '\t\t<title>Circa+Plastic API docs</title>\n'
    html += '\t\t<link rel="stylesheet" type="text/css" href="nanodoc.css" />\n'
    html += '\t\t<script type="text/javascript" src="nanodoc.js"></script>'
    html += "\t</head>\n"
    html += "\t<body>\n"
    html += '\t\t<div id="nanohead"><div id="nanotitle">' + doc['headers']['title'] + ' API documentation'
    
    html += """<span id="navigation_panel">
    <span><a href="../index.html">home</a></span>
    <span id="selected">docs</span>
    <span><a href="http://github.com/andyfischer/circa">github home</a></span>
    <span></span>
    </div>"""
    
    html += '</div></div>\n'

    html += '<div id="credit_link">This page was created with a modified version of <a href="http://code.google.com/p/nanodoc/">nanodoc</a></div>'

    html += '\t\t<div id="nanobody">\n'

    html += packageListHTML(all_packages)

    # 2nd and 3rd columns: contents of packages
    for package in all_packages:
        package_name = package['name']

        html += ("<div style=\"display:none\" class=\"package_contents\" id=\"%s_contents\">"
                % to_ident(package_name))
        html += "<div id=\"modulestitle\">%s</div>" % package_name
        html += "<div id=\"nanobutton\">"

        # split up contents into 2 lists
        contents = list(package['contents'])

        contents.sort(lambda lhs,rhs: cmp(lhs['name'], rhs['name']))

        contents_midpoint = len(contents)/2 + 1
        left_list = contents[:contents_midpoint]
        right_list = contents[contents_midpoint:]

        html += '<div class="package_contents_bar" style="float:left">\n'
        html += itemListHTML(left_list)
        html += "</div>"

        html += '<div class="package_contents_bar" style="float:right">\n'
        html += itemListHTML(right_list)
        html += "</div>"

        html += "</div>"
        html += "</div>"

        html += "\n"
    html += "\n"

    # Rightmost item: actual documentation
    for func in all_functions():
        html += detailHTML(func)
        html += '\n'

    html += "\n"

    if False:
      for item in doc.documentElement.getElementsByTagName('struct'):
        html += "<div style=\"display:none\" class=\"moduledetail\" id=\"%s_sub\">" % (item.attributes["name"].value.strip(" ").lower())
        html +=    "<div id=\"moduledetailtitle\">%s</div>" % (item.attributes["name"].value)
        html += "<div id=\"detailcontent\"> <span class=\"hl_type\">typedef struct</span> {<br/>"
        paramdescs = ""
        if len(item.getElementsByTagName('param')) > 0:
            params = []
            for subitem in item.getElementsByTagName('param'):
                if subitem.attributes["isPtr"].value == "True":
                    ptrFlag = "*"
                else:
                    ptrFlag = ""
                params.append("<span class=\"hl_type\">%s</span> %s<span class=\"hl_param\">%s</span>" % (subitem.attributes["type"].value, ptrFlag, subitem.attributes["name"].value))
                paramdescs += "<div class=\"hl_type_b\">%s</div><div class=\"ptr\">%s</div><div class=\"hl_param_b\">%s</div><div class=\"hl_paramdesc\">%s</div>" % (subitem.attributes["type"].value, ptrFlag, subitem.attributes["name"].value, subitem.childNodes[0].data)
            html += ";<br/> ".join(params)
        html += ";<br/>} %s;</div>" % (item.attributes["name"].value)
        
        if len(item.getElementsByTagName('desc')) > 0:
            html +=    "<div id=\"detailcontent\"><div class=\"detailboxdesc\"> Description</div> <div class=\"detailbox\">%s.</div></div>" % (item.getElementsByTagName('desc')[0].childNodes[0].data)
        
        if len(paramdescs) > 0:
            html += "<div id=\"detailcontent\"><div class=\"detailboxdesc\"> Parameters</div><div class=\"detailbox\">"
            html += paramdescs
            html += "</div></div>"
        html += "</div>"


    html += "\t\t<div>\n"
    html += "\t</body>\n"
    html += "\t</html>\n"
    return html



def write_text_file(filename, contents):
    output_file = open(filename, 'w')
    output_file.write(contents)
    output_file.write("\n")

html = makeHTML(Doc)
write_text_file(cl_options.output_filename, html)
