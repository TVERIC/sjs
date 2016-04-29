
const _path = require('_path');


function join() {
    if (arguments.length === 0)
        return '.';
    var joined;
    for (var i = 0; i < arguments.length; ++i) {
        var arg = arguments[i];
        if (arg.length > 0) {
            if (joined === undefined) {
                joined = arg;
            } else {
                var sep = '';
                if (arg.startsWith('/')) {
                    arg = arg.substring(1);
                }
                if (!joined.endsWith('/')) {
                    sep = '/';
                }
                joined += sep + arg;
            }
        }
    }
    if (joined === undefined)
        return '.';
    return joined;
}


module.exports = {
    basename: _path.basename,
    dirname: _path.dirname,
    normalize: _path.normalize,
    join: join
};
