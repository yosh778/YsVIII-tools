/**
 * Example parser and rebuilder of binary string table files, found in YS8.
 */

var fs = require("fs");
var jconv = require("jconv");

var input = null;
var output = null;
var mode = "help";
var enc = "utf8";

if (process.argv[2] == "pack") {
    mode = "pack";
} else if (process.argv[2] == "unpack") {
    mode = "unpack";
} else {
    console.error("Invalid mode specified.");
}

for (var i = 3; i < process.argv.length; i++){
    let arg = process.argv[i];
    if (arg == "--enc") {
        enc = process.argv[++i];
    } else {
        if (input == null) {
            input = arg;
        } else {
            output = arg;
        }
    }
}

if (input == null) {
    console.error("No input file specified.");
    mode = "help";
} else if (output == null) {
    console.error("No output file specified.");
    mode = "help";
} else if (!enc) {
    enc = "utf8";
    console.error("Using default encoding: utf8.");
}

switch (mode) {
    case "pack":
        packFile(input, output, enc);
        break;
    case "unpack":
        unpackFile(input, output, enc);
        break;
    case "help":
        console.error("Usage:\n\tnode tbbconv.js pack|unpack [--enc <encoding>] input output\n");
        break;    
}

function packFile(src, dst, enc) {
    //Pack CSV into bin
    
    var csv = fs.readFileSync(src, "utf8");

    //Split by rows and columns removing empty lines
    var csvl = csv.split(/\r\n|\r|\n/);
    for (var i = 0; i < csvl.length; i++){
        if (csvl[i].length == 0) {
            csvl.splice(i, 1);
            i--;
            continue;
        }
        csvl[i] = csvl[i].split("\t");
    }

    var cols = csvl[0].length;
    var rows = csvl.length;
    var count = cols * rows;

    console.log("Encoding CSV file into TBB.");
    console.log(`${count} strings in ${rows} rows and ${cols} columns.`);

    var ofd = fs.openSync(dst, "w");

    var head = Buffer.alloc(12);
    head.writeUInt32LE(cols, 0);
    head.writeUInt32LE(rows, 4);

    //Write header
    fs.writeSync(ofd, head, 0, head.length);

    var strenc = function(string) {
        return Buffer.from(string, enc);
    }
    if (enc == "shift-jis") {
        strenc = function(string) {
            return jconv.encode(string, "SJIS");
        }
    }

    //Begin writing strings
    var blocklen = 0;
    var offsets = Buffer.alloc(count * 4);
    for (var r = 0; r < rows; r++){
        for (var c = 0; c < cols; c++){
            //Write offset
            offsets.writeUInt32LE(blocklen, (r * cols + c) * 4);

            //Write string
            var b = strenc(csvl[r][c] + "\x00");
            fs.writeSync(ofd, b, 0, b.length);
            
            //Advance offset
            blocklen += b.length;
        }
    }

    console.log(`Wrote ${blocklen} bytes strings table.`);

    fs.writeSync(ofd, offsets, 0, offsets.length);

    console.log(`Wrote ${offsets.length} bytes offset table.`);

    //Write corrected header
    head.writeUInt32LE(blocklen, 0x8);
    fs.writeSync(ofd, head, 0, head.length, 0);

    fs.closeSync(ofd);
}

function unpackFile(src, dst, enc) {
    //Unpack bin to CSV
    var buf = fs.readFileSync(src);

    var cols = buf.readUInt32LE(0x0);
    var rows = buf.readUInt32LE(0x4);
    var len = buf.readUInt32LE(0x8);

    var strings = buf.slice(0xC, len + 0xC);
    var offsets = buf.slice(0xC + len);

    //Convert offsets
    {
        var a = new Array(offsets.length / 4);
        for (var i = 0; i < offsets.length; i += 4){
            a[i / 4 | 0] = offsets.readUInt32LE(i);
        }
        offsets = a;
    }

    var count = cols * rows;

    console.log("Opened binary TBB file.");
    console.log(`String table: ${strings.length} bytes.`);
    console.log(`${count} strings in ${rows} rows and ${cols} columns.`);

    var ofd = fs.openSync(dst, "w");

    var strdec = function(buf, start, end) {
        return buf.toString(enc, start, end);
    }
    if (enc == "shift-jis") {
        strdec = function(buf, start, end) {
            return jconv.decode(buf.slice(start, end), "SJIS");
        }
    }
    
    for (var r = 0; r < rows; r++){
        let out = "";
        for (var c = 0; c < cols; c++) {
            let idx = r * cols + c;
            let next = a[idx];
            while (strings[next++] != 0);
            //Cut out null terminator
            out += strdec(strings, a[idx], next-1) + "\t";
        }
        out = out.substr(0, out.length - 1) + "\r\n";
        fs.writeSync(ofd, out, null, "utf8");
    }

    fs.close(ofd);
}

