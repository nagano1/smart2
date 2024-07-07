console.log('Gulp starting...')

const path = require('path');
const gulp = require('gulp')
const fs = require('fs')
const util = require('util')
const exec = require('child_process').exec;
const del = require('delete')
const glob = require('glob')
const chokidar = require('chokidar');


//const username = require('username')
const isWin = /^win/.test(process.platform)
const sep = isWin ? '\\' : '/'

const altCmake = require('./alt_cmake');

const promise = readyWindowsCommandPrompt()

gulp.task("show", async function (cb) {
    await promise;
    
    const val = await altCmake.dev()
    
    console.info(val)
    cb();
});

gulp.task("s", async function (cb) {
    await promise;

    // find msbuild path
    // https://stackoverflow.com/questions/328017/path-to-msbuild
    let msbuildPath = await execAndReturn(`"%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\\**\\Bin\\MSBuild.exe`);
    if (msbuildPath == null || msbuildPath.trim() == "") {
        console.info(`couldn't find a msbuild path`);
    }

    let exec = `C:\\GitProjects\\doorlang_root\\other_projects\\visual_studio_console_sln\\RunSampleB\\x64\\Debug\\RunSampleB.exe`;

    //let projPath = `../other_projects/visual_studio_console_sln/ConsoleApplication2/ConsoleApplication2.vcxproj`;
    //let projPath = `../other_projects/visual_studio_console_sln/StaticLib1/StaticLib1.vcxproj`;
    let projPath = `../other_projects/visual_studio_console_sln/RunSampleB/RunSampleB.vcxproj`;
    let includePath = `$(ProjectDir)/../../../../src;$(ProjectDir)/../../../../src/include;$(IncludePath)`;
    let option = `/v:m /p:Configuration=Debug /p:Platform=x64 /p:IncludePath="${includePath}"`;

    let isBuilding = false;
    let isBuildRequested = false;


    chokidar.watch("../src/**/*").on('all', async (event, file) => {
        console.info(`watch event: ${event}, file: ${file}`); //event: add, change

        if (isBuilding) {
            isBuildRequested = true;
            return;
        }

        await buildUsingMsBuild(event);
    });

    async function buildUsingMsBuild(event) {

        isBuilding = true;

        console.log();
        console.log("---------------- Build ----------------");

        let date = Date.now();
        let error = await doExecAsyncWithHighlightedOutput(`"${msbuildPath}" "${projPath}" ${option}`);

        if (error) {
            let s = '\x1b[31mError\x1b[0m';
            console.error(`---------------- ${s} ----------------`);
        } else {
            let s = '\x1b[32mComplete\x1b[0m';
            let span = Date.now() - date;
            console.log(`---------------- ${s} ${span}ms ----------------`);

            let result = await doExecAsyncWithHighlightedOutput(`${exec}`);
            console.log(`---------------- Result ----------------`);

        }
        isBuilding = false;
        
        if (isBuildRequested) {
            isBuildRequested = false;
            await buildUsingMsBuild(event);
        }
    }
});

// watch and generate a test info json file
gulp.task("watchtest", async function (cb) {
    await promise;1

    collectTestList();

    var gaze_opt = {
        debounceDelay: 1000 // wait 1 sec after the last run
    }

    gulp.watch([
            `../tests/google_tests/*.cc`,
            `../tests/google_tests/*.h`,
            `../tests/google_tests/*.cpp`,
            `../tests/google_tests/*.hh`,
        ],
        gaze_opt,
        async cb => {
            collectTestList()
        }
    )
});


function collectTestList() {

    const testcppfiles = [path.posix.join("..", "tests", "google_tests")];

    const testlist = []

    for (let testdir of testcppfiles) {
        try {
            const files = fs.readdirSync(testdir)

            files.filter(function (file) {
                return /.*\.cpp$/.test(file);
            }).forEach(function (file) {
                let buffer = fs.readFileSync(path.join(testdir, file))
                let text = buffer.toString()

                let currentIndex = 0;
                while (true) {
                    const testIndex = text.indexOf("TEST(", currentIndex)
                    if (testIndex > -1) {
                        const testEndIndex = text.indexOf("ENDTEST", testIndex + 2)
                        const nextTestIndex = text.indexOf("TEST(", testIndex + 2)
                        if (
                            (nextTestIndex == -1 || testEndIndex < nextTestIndex) && testEndIndex > -1) {
                            console.info("========== TEST ==========\n" + text.substring(testIndex, testEndIndex))

                            const testText = text.substring(testIndex, testEndIndex)
                            const indexOfEndParenthes = testText.indexOf(")")

                            const terms = testText.substring(5, indexOfEndParenthes).split(',')

                            if (terms.length > 1) {
                                testlist.push(
                                    {
                                        testcasename: terms[0].trim(),
                                        testname: terms[1].trim(),
                                        filepath: path.posix.join(testdir, file),
                                        filename: file,
                                        body: testText
                                    }
                                )
                            }
                        }
                        currentIndex = testIndex + 2
                    } else {
                        break;
                    }
                }
            });

        } catch (err) {
            console.error(err)
        }
    }

    fs.writeFileSync(
        path.join("..", "other_projects", "AndroidCanLang", "app", "src", "main", "res", "raw", "testlist.json")
        , JSON.stringify(testlist, null, "\t")
    )

    console.info(testlist)
}










// --------- util functions ---------

function line(str) {
    return str.split('\n').join(' ')
}

function doExec(str, cb) {
    child = exec(line(str), (error, stdout, stderr) => {
        // console.log(error);
        // console.log(stderr || stdout);
        cb(error)
    })

    child.stdout.addListener('data', d => {
        //var BgRed = "\x1b[41m";
        let s = '\x1b[31mError\x1b[0m';
        let subst = d.toString().replaceAll("error", s);
    
        console.log(subst)
    })
    child.stderr.addListener('data', d => {
        console.log(d)
    })
}

async function doExecAsyncWithHighlightedOutput(str) {
    return new Promise((resolve, reject) => {
        child = exec(line(str), (error, stdout, stderr) => {
            resolve(error)
        })

        child.stdout.addListener('data', d => {
            let s = '\x1b[31merror\x1b[0m';
            let subst = d.toString().replaceAll("error", s);
            let s2 = '\x1b[31mError\x1b[0m';
            subst = subst.replaceAll("Error", s2);
            console.log(subst)
        })
        child.stderr.addListener('data', d => {
            console.log(d)
        })
    })
}

async function execAndReturn(str) {
    return new Promise((resolve, reject) => {
        child = exec(line(str), (error, stdout, stderr) => {
            resolve(error)
        })

        child.stdout.addListener('data', d => {
            resolve(d);
        })
        child.stderr.addListener('data', d => {
        })
    })
}



async function readyWindowsCommandPrompt() {
    if (isWin) {
        await doExecAsyncWithHighlightedOutput(`chcp 65001`)
    }
}


//gulp.task('default', ['typescript'])
//gulp.task('default', ['typescript', 'watch']);