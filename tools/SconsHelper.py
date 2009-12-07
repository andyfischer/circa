
class EnvironmentSet():
    """
    This class wraps around a set of Environments. You can make normal
    Environment calls on it, and each call will be dispatched to each
    env. There is also some support for string templating using build
    names. We use this class for split debug/release builds.
    """
    def __init__(self, **namedEnvs):
        self.namedEnvs = namedEnvs
    def templateString(self, str, name):
        return str.replace("$buildname", name)
    def Clone(self):
        clonedEnvs = {}
        for (name, env) in self.namedEnvs.items():
            clonedEnvs[name] = env.Clone()
        return EnvironmentSet(**clonedEnvs)
    def Append(self, *args, **kwargs):
        for env in self.namedEnvs.values():
            env.Append(*args, **kwargs)
    def SetOption(self, *args, **kwargs):
        for env in self.namedEnvs.values():
            env.SetOption(*args, **kwargs)
    def BuildDir(self, buildDir, sourceDir):
        for (name,env) in self.namedEnvs.items():
            env.BuildDir(self.templateString(buildDir, name), sourceDir)
    def StaticLibrary(self, libraryName, sourceList):
        results = {}
        for (name, env) in self.namedEnvs.items():
            _libraryName = self.templateString(libraryName, name)
            _sourceList = map(lambda s: self.templateString(s, name), sourceList)
            results[name] = env.StaticLibrary(_libraryName, _sourceList)
        return results
    def __getitem__(self, item):
        return self.namedEnvs[item]

def download_file_from_the_internets(url, filename):
    print "Downloading "+url
    import urllib
    webFile = urllib.urlopen(url)
    localFile = open(filename, 'wb')
    localFile.write(webFile.read())
    webFile.close()
    localFile.close()

def unzip_file(filename, dir):
    print "Unzipping "+filename+" to "+dir
    import os, zipfile

    if not os.path.exists(dir): os.mkdir(dir)

    zf = zipfile.ZipFile(filename)
    for name in zf.namelist():
        path = os.path.join(dir, name)
        if name.endswith('/'):
            if not os.path.exists(path): os.mkdir(path)
        else:
            f = open(path, 'wb')
            f.write(zf.read(name))
            f.close()

