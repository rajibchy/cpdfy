console.log(process.pid);
const path = require('path');
const os = require("os");
const platform = os.platform();
const arch = os.arch();
//if (platform !== 'win32' || arch !== 'ia32') throw new Error(`Not supported platform =>${platform} and arch => ${arch}`);
console.log(`platform =>${platform} and arch => ${arch}`);
const { html2pdf } = require('./index');
const fs = require('fs');
console.log(html2pdf.getHttpHeader());
const sleep = require('util').promisify(setTimeout)
async function test() {
    //html2pdf.generatePdf({ from_url: "https://wkhtmltopdf.org/", out_path: path.resolve('./from_url.pdf') });
    //await sleep(5);
    const html = fs.readFileSync('./test_output/test.html', { encoding: "utf-8" }).replace(/^\uFEFF/, '');
    const xhtml = `<!DOCTYPE html>
    <html lang="es">
    <head>
        <title>Test PDF</title>
    </head>
    <BODY>
        <h1 style="color:red;">Hello World....</h1>
    </BODY>
    </html>
    `;
    await sleep(1000);
    console.log(os.tmpdir());
    const fst = fs.createWriteStream(path.resolve(`./test_output/test_${Math.floor((0x999 + Math.random()) * 0x10000000)}.pdf`));
    html2pdf.createStram({}, html).pipe(fst);
    return;
    for (let i = 0; i < 100; i++) {
        const result = html2pdf.generatePdf({ out_path: path.resolve(`./test_output/test_${Math.floor((0x999 + Math.random()) * 0x10000000)}.pdf`) }, html);
        console.log(`Result ${result}=>${i + 1}`);
        //if (global.gc) { global.gc(); }
        /*const pdfBuff = html2pdf.generatePdf({ }, html);
        if (Buffer.isBuffer(pdfBuff)) {
            if (pdfBuff.length > 10) {
                require('fs').writeFileSync(`./test_output/test_${Math.floor((0x999 + Math.random()) * 0x10000000)}.pdf`, pdfBuff);
                console.log(`Success=>${i + 1}`);
            } else {
                console.log(pdfBuff.toString() + '=>' + i + 1);
            }

        } else {
            console.log(pdfBuff);
        }*/

        //await sleep(500);
    }

}
test();
// call "D:\\Program Files\\nodejs\\node.exe" test.js