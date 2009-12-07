
"""

Json document is layed out like this:

headers
  title
packages
  name
  contents

"""

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

def compareItems(lhs, rhs):
    return cmp(lhs['name'], rhs['name'])

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
    html += '<span class="detail_declaration">%s</span>\n' % item['declaration']
    html += '<p>'
    html += '<span class="detail_comments">%s</span>\n' % item['comments']

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

        contents.sort(compareItems)

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

from optparse import OptionParser
options = OptionParser()
options.add_option('--doc', dest = 'doc_filename')
options.add_option('--output', dest = 'output_filename')
(cl_options, cl_args) = options.parse_args()

def load_json_doc(filename):
    f = open(filename, 'r')
    file_contents = f.read()
    f.close()

    import json
    doc = json.loads(file_contents)
    return doc

def write_text_file(filename, contents):
    output_file = open(filename, 'w')
    output_file.write(contents)
    output_file.write("\n")

doc = load_json_doc(cl_options.doc_filename)
html = makeHTML(doc)
write_text_file(cl_options.output_filename, html)
