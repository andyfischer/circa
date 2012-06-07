
DocsDump = 'docs.json'
DestinationDocs = 'gh-pages/docs'
DestinationIncludes = 'gh-pages/_includes/docs'

import json,math

# Load dumped data
docsDumpString = open(DocsDump).read()
# Hack, fix quote marks
docsDumpString = docsDumpString.replace('\'', '"')
everyEntry = json.loads(docsDumpString)

# Correlate data
termToEntry = {}
everyFunction = []
everyType = []

circa_allTypes = {'title':'All types', 'name':'all_types', 'items':[]}
circa_allFunctions = {'title':'All functions', 'name':'all_functions', 'items':[]}
improv_allTypes = {'title':'All types','name':'improv_types', 'items':[]}
improv_allFunctions = {'title':'All functions','name':'improv_functions','items':[]}

circaCategory = {'title':'Core','subcategories': [circa_allTypes, circa_allFunctions]}
improvCategory = {'title':'Improv','subcategories': [improv_allTypes, improv_allFunctions]}
majorCategories = [circaCategory, improvCategory]

# minor categories:
# Comparison
# Internal
# Math
# Reflection
# Stateful
# Uncategorized

def everySubCategory():
    for cat in majorCategories:
        for subcat in cat['subcategories']:
            yield subcat

for entry in everyEntry:
    if entry['type'] == 'Function':
        everyFunction.append(entry)

        circa_allFunctions['items'].append(entry)

        # Assign derived data
        entry['title'] = entry['name']
        entry['filename'] = DestinationDocs + '/' + entry['name'] + '.md'
        entry['url'] = '/docs/' + entry['name'] + '.html'
        entry['linkHtml'] = '<a href="' + entry['url'] + '">' + entry['title'] + '</a>'

    elif entry['type'] == 'Type':
        everyType.append(entry)

        circa_allTypes['items'].append(entry)

        entry['title'] = entry['name']
        entry['filename'] = DestinationDocs + '/' + entry['name'] + '.md'
        entry['url'] = '/docs/' + entry['name'] + '.html'
        entry['linkHtml'] = '<a href="' + entry['url'] + '">' + entry['title'] + '</a>'

    else:
        print "Didn't recognize type:", entry

    if 'term' in entry:
        if entry['term'] == "Term#null":
            continue
        termToEntry[entry['term']] = entry

for cat in everySubCategory():
    cat['filename'] = DestinationDocs + '/' + cat['name'] + '.md'
    cat['url'] = '/docs/' + cat['name'] + '.html'

def writeFunctionPage(func):
    out = []

    title = func['title']

    out.append("---")
    out.append("title: "+ title)
    out.append("layout: docs")
    out.append("---")
    out.append("")
    out.append("<h3><i>function</i> " + title + "</h3>")
    out.append("")

    heading = ""

    if 'heading' not in func:
        print "missing 'heading' from:", func['name']
        return []

    for element in func['heading']:
        term = element[1]
        enableLink = False

        if term != func['term'] and term in termToEntry:
            linkedEntry = termToEntry[term]
            enableLink = True
            heading += '<a href="' + linkedEntry['url'] + '">'

        heading += element[0]

        if enableLink:
            heading += '</a>'

    out.append(heading)
    out.append("")
    out.append("See also:")
    out.append("")
    out.append("Labels:")

    return out

def writeTypePage(t):
    out = []

    title = t['title']

    out.append("---")
    out.append("title: "+ title)
    out.append("layout: docs")
    out.append("---")
    out.append("")
    out.append("<h3><i>type</i> " + title + "</h3>")
    out.append("")
    out.append("Methods:")
    out.append("")

    return out

def writeLeftBar():
    out = []
    out.append('<div class="left_bar">')

    for majorCategory in majorCategories:
        out.append('<p><h1>' + majorCategory['title'] + '</h1></p>')

        for subcategory in majorCategory['subcategories']:
            out.append('  <p><h3><a href="' + subcategory['url'] + '">'
                + subcategory['title']
                + '</a></h3></p>')

    out.append('</div>')

    return out

def writeCategoryPage(category):
    title = category['title']

    out = []

    out.append("---")
    out.append("title: "+ title)
    out.append("layout: docs")
    out.append("---")
    out.append("")
    out.append("<h3>" + title + "</h3>")
    out.append("")

    # Divvy up functions into columns

    # Enforce a maximum of 4 columns
    MaxColumns = 4

    PreferredLimitPerColumn = 15

    items = category['items']
    items.sort(key = lambda i: i['title'].lower())

    columns = []

    maxItemsPerColumn = 0

    # If we can limit each column to 15 items or less, then minimize the column count
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

            out.append('    <td>'+ item['linkHtml'] + '</td>')
        out.append('  </tr>')
    out.append('</table>')
    out.append('')

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
    lines = writeCategoryPage(cat)
    writeFile(lines, cat['filename'])

# Function-specific pages
for func in everyFunction:
    lines = writeFunctionPage(func)
    writeFile(lines, func['filename'])

# Type-specific pages
for t in everyType:
    lines = writeTypePage(t)
    writeFile(lines, t['filename'])
