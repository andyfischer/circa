
DestinationDocs = 'gh-pages/docs'
DestinationIncludes = 'gh-pages/_includes/docs'

import json,math

everyEntry = []

# Load dumped data
def load_json_file(filename):
    return json.loads(open(filename).read())

for entry in load_json_file('circa.docs.json'):
    entry['source'] = 'circa'
    everyEntry.append(entry)
for entry in load_json_file('improv.docs.json'):
    entry['source'] = 'improv'
    everyEntry.append(entry)

# Correlate data
termToEntry = {}
nameToEntry = {}

circa_allTypes = {'title':'All types', 'name':'all_types', 'items':[]}
circa_allFunctions = {'title':'All functions', 'name':'all_functions', 'items':[]}
circaCategory = {'title':'Core',
    'type':'Category',
    'subcategories': [circa_allTypes, circa_allFunctions]}

improv_allTypes = {'title':'All types','name':'improv_types', 'items':[]}
improv_allFunctions = {'title':'All functions','name':'improv_functions','items':[]}
improvCategory = {'title':'Improv',
    'type':'Category',
    'subcategories': [improv_allTypes, improv_allFunctions]}
majorCategories = [circaCategory, improvCategory]

for title in [
        '#circa',
        'Containers',
        'Internal',
        'Logic',
        'Math',
        'Randomness',
        'Reflection',
        'Stateful',
        ['Uncategorized', 'core_Uncategorized'],
        '#improv',
        'Drawing',
        'Environment',
        'Input',
        'Text',
        'UI Widgets',
        ['Uncategorized', 'improv_Uncategorized']
        ]:


    if type(title) == list:
        name = title[1]
        title = title[0]
    else:
        name = title

    if title == '#circa':
        majorCategory = circaCategory
        continue
    elif title == '#improv':
        majorCategory = improvCategory
        continue

    name = name.replace(' ', '_')
    category = {'title':title, 'name':name, 'items':[]}
    majorCategory['subcategories'].append(category)
    nameToEntry[category['name']] = category

def everySubCategory():
    for cat in majorCategories:
        for subcat in cat['subcategories']:
            yield subcat


for entry in everyEntry:

    # Append to All Functions or All Types
    if entry['type'] == 'Function':
        if entry['source'] == 'circa':
            category = circa_allFunctions
        else:
            category = improv_allFunctions

        category['items'].append(entry)
    elif entry['type'] == 'Type':
        if entry['source'] == 'circa':
            category = circa_allTypes
        else:
            category = improv_allTypes
        category['items'].append(entry)

    # Assign derived data

    # Fix names that will collide when they are case-insensitive
    nameForUrl = entry['name']
    if entry['name'] == 'list':
        nameForUrl = 'list_func'
    elif entry['name'] == 'set':
        nameForUrl = 'set_func'
    elif entry['name'] == 'list':
        nameForUrl = 'list_func'
    elif entry['name'] == 'branch':
        nameForUrl = 'branch_func'
    elif entry['name'] == 'type':
        nameForUrl = 'type_func'
    elif entry['name'] == 'rect':
        nameForUrl = 'rect_func'
    elif entry['name'] == 'button':
        nameForUrl = 'button_func'

    entry['title'] = entry['name']
    entry['filename'] = DestinationDocs + '/' + nameForUrl + '.md'
    entry['url'] = '/docs/' + nameForUrl + '.html'
    entry['linkHtml'] = '<a href="' + entry['url'] + '">' + entry['title'] + '</a>'
    entry['belongsToCategories'] = []
    entry['overloadEntries'] = []
    entry['typeMethods'] = []

    if 'tags' not in entry:
        entry['tags'] = []

    if 'term' in entry:
        if entry['term'] == "Term#null":
            continue
        termToEntry[entry['term']] = entry

    if 'name' in entry:
        nameToEntry[entry['name']] = entry

# Correlate overloaded functions with their contents
for entry in everyEntry:
    if 'isOverloaded' in entry and entry['isOverloaded']:
        for term in entry['overloads']:
            if term not in termToEntry:
                print "couldn't find ref", term, "for overloaded func", entry['name']
                continue

            overloadEntry = termToEntry[term]
            entry['overloadEntries'].append(overloadEntry)

# Correlate methods with their owning types
for entry in everyEntry:
    if 'isMethod' in entry and entry['isMethod']:
        owningType = entry['inputTypes'][0]
        typeEntry = termToEntry[owningType]
        entry['methodOfType'] = typeEntry
        typeEntry['typeMethods'].append(entry)

# Function header HTML
for entry in everyEntry:
    if 'heading' in entry:
        heading = ""
        # the cross-referenced heading is used when we're on a different page than this
        # function. The function's name has a link to its page.
        crossReferencedHeading = ""
        
        for element in entry['heading']:
            firstElement = element == entry['heading'][0]
            term = element[1]
            enableLink = False

            if term != entry['term'] and term in termToEntry:
                linkedEntry = termToEntry[term]
                enableLink = True
                heading += '<a href="' + linkedEntry['url'] + '">'

            heading += element[0]

            # cross-reference: only first element is a link
            crossReferenceLink = False
            if firstElement:
                linkStart = '<a href="' + entry['url'] + '">'
                crossReferencedHeading += linkStart
                crossReferenceLink = True

            # if this is a method, then lighten the type name to decrease its importance.
            if firstElement and entry['isMethod']:
                split = element[0].split('.')
                crossReferencedHeading += '<span class="minor">' + split[0] + '</span>'
                crossReferencedHeading += '.' + split[1]
            else:
                crossReferencedHeading += element[0]

            if enableLink:
                heading += '</a>'
            if crossReferenceLink:
                crossReferencedHeading += '</a>'

        entry['headingHTML'] = heading
        entry['crossReferencedHeading'] = crossReferencedHeading
    
# Add some descriptions
nameToEntry['number']['description'] = 'Floating-point number type'
nameToEntry['any']['description'] = 'Catch-all type, used for runtime typing'
nameToEntry['bool']['description'] = 'Boolean type, either true or false.'
nameToEntry['Branch']['description'] = 'Reference to a section of code. Each Branch contains a list of Terms (which may themselves have a nested Branch). A Branch may have an \'owner\' Term.'
nameToEntry['Dict']['description'] = 'Dictionary type, associates string keys with arbitrary values.'
nameToEntry['Frame']['description'] = 'A Frame is a single entry on an Interpreter\'s call stack. Each Frame contains a reference to the Branch that it uses, and a list of active register values.'
nameToEntry['List']['description'] = 'List type, contains an ordered list of elements.'
nameToEntry['String']['description'] = 'String type, holds a sequence of characters.'
nameToEntry['Interpreter']['description'] = 'An interpreter is an object which executes Circa code.'
nameToEntry['Map']['description'] = 'Map type, stores an associated value for a key'
nameToEntry['Point']['description'] = 'Point type, stores a two-dimensional vector.'
nameToEntry['Rect']['description'] = 'Rect type, stores the position of a rectangle.'

nameToEntry['Internal']['description'] = 'Internally-used functions. These should not be called directly.'
nameToEntry['Term']['description'] = 'A single unit of code. Each Term has a function, and 0 or more input Terms. A Term may have a local name. Every term has a Branch parent, and a term may optionally have a nested Branch.'
nameToEntry['Randomness']['description'] = 'Functions that use randomness.'
nameToEntry['Reflection']['description'] = 'Functions and types for inspecting and modifying Circa\'s internals, including the interpreter and code data.'
nameToEntry['Stateful']['description'] = 'Functions that use inlined state.'

# Derived data on categories
for cat in everySubCategory():
    cat['type'] = 'Category'
    cat['filename'] = DestinationDocs + '/' + cat['name'] + '.md'
    cat['url'] = '/docs/' + cat['name'] + '.html'
    cat['linkHtml'] = '<a href="' + cat['url'] + '">' + cat['title'] + '</a>'

# Manually put functions into categories.
def addToCategory(entry, category):
    category['items'].append(entry)
    entry['belongsToCategories'].append(category)

def addNamesToCategory(nameList, categoryName):
    category = nameToEntry[categoryName]
    for name in nameList:
        if name not in nameToEntry:
            print("couldn't find name:", name)
            continue

        entry = nameToEntry[name]
        addToCategory(entry, category)

# Sort items into categories

#addNamesToCategory(['Color','blend_color','random_color','hsv_to_rgb'], 'color_tag')
#addNamesToCategory(['darken','lighten'], 'color_tag')

addNamesToCategory(['rand','rand_i','rand_range'], 'Randomness')
addNamesToCategory(['seed','random_color','random_element','random_norm_vector'], 'Randomness')

addNamesToCategory(['abs','add','div','sub','ceil'], 'Math')
addNamesToCategory(['arcsin', 'arccos', 'arctan', 'average', 'bezier3', 'bezier4'], 'Math')
addNamesToCategory(['floor', 'log', 'magnitude'], 'Math')
addNamesToCategory(['max','min','mod'], 'Math')
addNamesToCategory(['mult', 'neg','pow','rand','seed'], 'Math')
addNamesToCategory(['sin', 'sqr', 'sqrt', 'cos','tan','rotate_point','round'], 'Math')
addNamesToCategory(['remainder','clamp','polar', 'point_distance'], 'Math')
addNamesToCategory(['smoothstep','smootherstep'], 'Math')
addNamesToCategory(['div_f','div_i','div_s'], 'Math')
addNamesToCategory(['Point','Point_i','Rect','Rect_i'], 'Math')
addNamesToCategory(['smooth_in_out','cube'], 'Math')
addNamesToCategory(['rand_i','rand_range','random_norm_vector'], 'Math')

addNamesToCategory(['equals','not_equals','greater_than', 'greater_than_eq'], 'Logic')
addNamesToCategory(['less_than','less_than_eq'], 'Logic')
addNamesToCategory(['and','or','not'], 'Logic')
addNamesToCategory(['cond','any_true'], 'Logic')

addNamesToCategory(['Dict','Set','List','list','set'], 'Containers')

addNamesToCategory(['approach','approach_rect','delta','seed','toggle'], 'Stateful')
addNamesToCategory(['cycle','cycle_elements','once','changed'], 'Stateful')

addNamesToCategory(['extra_output','unknown_function','unknown_identifier'], 'Internal')
addNamesToCategory(['pack_state','unpack_state','pack_state_list_n','unpack_state_list_n'], 'Internal')
addNamesToCategory(['unpack_state_from_list','pack_state_to_list'], 'Internal')
addNamesToCategory(['unbounded_loop_finish','for'], 'Internal')
addNamesToCategory(['case','default_case','comment'], 'Internal')
addNamesToCategory(['input_placeholder','output_placeholder'], 'Internal')
addNamesToCategory(['test_oracle','test_spy'], 'Internal')
addNamesToCategory(['branch','declared_state','exit_point'], 'Internal')
addNamesToCategory(['static_error'], 'Internal')
addNamesToCategory(['switch','namespace','if_block'], 'Internal')
addNamesToCategory(['continue','break','discard','return'], 'Internal')
addNamesToCategory(['loop','loop_output','loop_iterator','loop_index'], 'Internal')
addNamesToCategory(['unrecognized_expr', 'overload_error_no_match'], 'Internal')
addNamesToCategory(['inputs_fit_function'], 'Internal')

# Internal debug:
addNamesToCategory(['debug_get_term_stack','dump_current_branch','dump_parse'], 'Internal')

addNamesToCategory(['Branch','Term','Interpreter','Frame','Type'], 'Reflection')
addNamesToCategory(['make_interpreter','overload:get_contents'], 'Reflection')
addNamesToCategory(['lookup_branch_ref','branch_ref','term_ref'], 'Reflection')
addNamesToCategory(['is_overloaded_func'], 'Reflection')

addNamesToCategory(['Button', 'ButtonBar','button','button_bar'], 'UI_Widgets')
addNamesToCategory(['Painter', 'PainterPath','PaintEvent','Pen'], 'Drawing')
addNamesToCategory(['Brush','create_brush','create_linear_gradient'], 'Drawing')
addNamesToCategory(['Environment'], 'Environment')
addNamesToCategory(['InputEvent'], 'Input')
addNamesToCategory(['keycode_to_name'], 'Input')
addNamesToCategory(['Font','FontMetrics','create_font'], 'Text')
addNamesToCategory(['Painter.drawText','Painter.setFont'], 'Text')


# For any overloaded function in a category, add its overloads to the same category
for entry in everyEntry:
    for overload in entry['overloadEntries']:
        for category in entry['belongsToCategories']:
            addToCategory(overload, category)

# For any type in a category, add its methods to the same category
for entry in everyEntry:
    for method in entry['typeMethods']:
        for category in entry['belongsToCategories']:
            addToCategory(method, category)

# Anything uncategorized goes in Uncategorized
for entry in everyEntry:
    if not entry['belongsToCategories']:

        if entry['source'] == 'circa':
            category = nameToEntry['core_Uncategorized']
        else:
            category = nameToEntry['improv_Uncategorized']

        category['items'].append(entry)

# Insert the Index page
indexEntry = {'title':'API Documentation', 'name':'index',
    'url':'/',
    'type':'HighLevel',
    'filename':DestinationDocs + '/index.md'}
everyEntry.append(indexEntry)

# Check that no entries have duplicate filenames or urls
everyFilename = set()
everyUrl = set()

for entry in everyEntry:
    filename = entry['filename']
    url = entry['url']

    filename = filename.lower()
    url = url.lower()

    if url in everyUrl:
        print "duplicate url:", url
    if filename in everyFilename:
        print "duplicate filename:", filename

    everyFilename.add(filename)
    everyUrl.add(url)

def getHTMLForAlphebetizedEntryList(items):

    out = []

    buckets = {}

    for item in items:
        title = item['title'].upper()
        letter = title[0]

        if letter not in buckets:
            buckets[letter] = []

        buckets[letter].append(item)

    letters = buckets.keys()
    letters.sort()

    for letter in letters:
        items = buckets[letter]
        out.append("<p>" + letter + "</p>")
        out.append("")
        out.append("<p>")

        out.append(", ".join([item['linkHtml'] for item in items]))

        out.append("</p>")

    return out

def getTableHTMLForEntryList(items):

    # If we have way too many items then create an alphebetized list instead.
    MinItemsForAlphebetizedList = 200

    if len(items) >= MinItemsForAlphebetizedList:
        return getHTMLForAlphebetizedEntryList(items)
    
    out = []

    # Enforce a maximum of 4 columns
    MaxColumns = 4

    PreferredLimitPerColumn = 10

    columns = []

    maxItemsPerColumn = 0

    # If we can limit each column to 10 items or less, then minimize the column count
    if len(items) <= MaxColumns * PreferredLimitPerColumn:
        maxItemsPerColumn = PreferredLimitPerColumn

    else:
        # Otherwise, evenly balance the items per column
        maxItemsPerColumn = int(math.ceil(1.0 * len(items) / MaxColumns))

    for index in range(len(items)):
        columnIndex = index / maxItemsPerColumn
        if columnIndex >= len(columns):
            columns.append([])
        columns[columnIndex].append(items[index])

    out.append('')
    out.append('<table>')

    for index in range(maxItemsPerColumn):
        out.append('  <tr>')
        for column in columns:
            if index >= len(column):
                continue

            item = column[index]

            if 'linkHtml' not in item:
                print 'no linkHtml in', item['name']
                continue

            out.append('    <td>'+ item['linkHtml'] + '</td>')
        out.append('  </tr>')
    out.append('</table>')
    out.append('')

    return out

def writePage(entry):
    out = []

    title = entry['title']

    out.append("---")
    out.append("title: "+ title)
    out.append("layout: docs")
    out.append("---")
    out.append("")

    if entry['type'] == 'Function':
        out.append('<h3><span class="minor">function</span> ' + title + "</h3>")
        out.append("")
    elif entry['type'] == 'Type':
        out.append('<h3><span class="minor">type</span> ' + title + "</h3>")
        out.append("")

    if 'headingHTML' in entry:
        out.append(entry['headingHTML'])

    # Doc comments
    if 'topComments' in entry:
        comments = []
        for comment in entry['topComments']:
            if comment.startswith('--'):
                comment = comment[2:]
            comment = comment.strip()
            comments.append(comment)
        out.append("<p>" + " ".join(comments) + "</p>")
        out.append("")

    if 'description' in entry:
        out.append('<p>' + entry['description'] + '</p>')

    # Category contents
    if entry['type'] == 'Category':
        types = []
        functions = []
        for item in entry['items']:
            if item['type'] == 'Type':
                types.append(item)
            else:
                functions.append(item)

        # Divvy up functions into columns
        types.sort(key = lambda i: i['title'].lower())
        functions.sort(key = lambda i: i['title'].lower())

        if types and functions:
            out.append('<h5>Types</h5>')
        if types:
            out.extend(getTableHTMLForEntryList(types))
            out.append("")
        if types and functions:
            out.append('<h5>Functions</h5>')
        if functions:
            out.extend(getTableHTMLForEntryList(functions))
            out.append("")
        

    if 'typeMethods' in entry and entry['typeMethods']:
        out.append('<p>Methods:</p>')
        for method in entry['typeMethods']:
            out.append('<p>' + method['crossReferencedHeading'] + '</p>')
        out.append('')

    if 'overloadEntries' in entry and entry['overloadEntries']:
        out.append("")
        out.append("<p>Overloaded function, includes these specific functions:</p>")

        for entry in entry['overloadEntries']:
            out.append('<p>' + entry['crossReferencedHeading'] + '</p>')
        out.append("")

    if 'belongsToCategories' in entry and entry['belongsToCategories']:
        out.append("<p>tags: ")
        out.append(", ".join(category['linkHtml'] for category in entry['belongsToCategories']))
        out.append("</p>")

    if 'term' in entry:
        out.append('<p><span class="extra_minor">' + entry['term'] + "</span></p>")

    import datetime
    bottomNote = "Page generated at " + str(datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))
    out.append('<div class="bottom_right_note">' + bottomNote + '</div>')

    out.append('<br>')
    out.append('<br>')

    return out

def writeLeftBar():
    out = []
    out.append('<div class="left_bar">')

    for majorCategory in majorCategories:
        out.append('<p><h1>' + majorCategory['title'] + '</h1></p>')

        for subcategory in majorCategory['subcategories']:
            out.append('  <p><h3>' + subcategory['linkHtml'] + '</h3></p>')

    out.append('<br>')
    out.append('<br>')
    out.append('</div>')

    return out

def writeFile(lines, file):
    f = open(file, 'w')
    for line in lines:
        f.write(line)
        f.write('\n')
    f.close()

# Left bar
writeFile(writeLeftBar(), DestinationIncludes + '/left_bar.html')

# Category pages
for cat in everySubCategory():
    lines = writePage(cat)
    writeFile(lines, cat['filename'])

# Function-specific pages
for entry in everyEntry:
    lines = writePage(entry)
    writeFile(lines, entry['filename'])
