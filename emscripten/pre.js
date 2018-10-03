//Module.arguments = ["--debug", "--window"];
Module['noInitialRun'] = true;
Module.filePackagePrefixURL = '/';

function loadChildScript(name, then) {
    var js = document.createElement('script');
    js.onerror = function(err) {
      console.log(err);
      Module.print("Could not download " + name);
      Module.setStatus("Missing data file!");
    };
    if (then) js.onload = then;
    js.src = name;
    document.body.appendChild(js);
}

Module['preInit'] = function() {
    // load game data externally so we can update it independently
    Module['addRunDependency']('download_t/freedink-data-data.js');
    loadChildScript('/freedink-data-data.js', function() {
        Module['removeRunDependency']('download_t/freedink-data-data.js');
    });
    Module['addRunDependency']('download_t/soundfonts-data.js');
    loadChildScript('/soundfonts-data.js', function() {
        Module['removeRunDependency']('download_t/soundfonts-data.js');
    });

    // populate savegames
    Module['addRunDependency']('fs_dotdink');
    try {
        FS.mkdir('/home/web_user/.dink');
        FS.mount(IDBFS, {}, '/home/web_user/.dink');
    } catch(e) {
        Module.print("Could not create ~/.dink/");
    }
    FS.syncfs(true, function(err) {
        Module['removeRunDependency']('fs_dotdink');
        if (err) { console.trace(); console.log(err); }
    })
}

Module['onRuntimeInitialized'] = function() {
    document.getElementById('ID_ScriptInstallDecoy').disabled = 0;
    document.getElementById('ID_SavegamesExport').disabled = 0;

    // music can be added after the game starts
    loadChildScript('/freedink-data-bgm-ogg-data.js',
        function() { Module.setStatus('Downloading music...'); });

    _GET = {}
    location.search.substr(1).split('&').forEach(function(item) {
        _GET[item.split("=")[0]] = item.split("=")[1]
    });
    if (_GET['dmod']) {
        if (!_GET['dmod'].startsWith('http://') && !_GET['dmod'].startsWith('http://'))
            _GET['dmod'] = 'https://' + _GET['dmod']
        var xhr = new XMLHttpRequest();
        xhr.open('GET', _GET['dmod'], true);
        xhr.responseType = 'arraybuffer';
        xhr.onprogress = function(event) {
            var url = _GET['dmod'];
            var size = -1;
            if (event.total) size = event.total;
            if (event.loaded) {
                if (!xhr.addedTotal) {
                    xhr.addedTotal = true;
                    if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
                    Module.dataFileDownloads[url] = {
                        loaded: event.loaded,
                        total: size
                    };
                } else {
                    Module.dataFileDownloads[url].loaded = event.loaded;
                }
                var total = 0;
                var loaded = 0;
                var num = 0;
                for (var download in Module.dataFileDownloads) {
                    var data = Module.dataFileDownloads[download];
                    total += data.total;
                    loaded += data.loaded;
                    num++;
                }
                total = Math.ceil(total * Module.expectedDataFileDownloads/num);
                if (Module['setStatus']) Module['setStatus']('Downloading D-Mod... (' + loaded + '/' + total + ')');
            } else if (!Module.dataFileDownloads) {
                if (Module['setStatus']) Module['setStatus']('Downloading D-Mod...');
            }
        };
        xhr.onerror = function(event) {
            console.log(event);
            Module.print("Cannot download D-Mod. Maybe the download was blocked, see the JavaScript console for more information.");
            UISetState(true, false);
        }
        xhr.onload = function(event) {
            if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || (xhr.status == 0 && xhr.response)) {
                FS.writeFile('mine.dmod', new Uint8Array(xhr.response));
                DmodExtract();
                if (Module['setStatus']) Module['setStatus']('');
            } else {
                console.log(event);
                Module.print("Error while downloading " + xhr.responseURL
                             + " : " + xhr.statusText + " (status code " + xhr.status + ")");
            }
            UISetState(true, false);
        };
        xhr.send(null);
    } else { /* no D-Mod querystring */
        try {
            dmoddiz = new TextDecoder().decode(FS.readFile('/usr/local/share/dink/dink/dmod.diz'), {encoding:'ISO-8859-1'});
            Module.print(dmoddiz);
            Module.print("");
            Module.print("Welcome! Click Play, or load one of your D-Mods!");
        } catch(e) {
            Module.print("Could not read dmod.diz");
        }
        UISetState(true, false);
    }
}

Module['onExit'] = function() {
    UISetState(true, false);
}
