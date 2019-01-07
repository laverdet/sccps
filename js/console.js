'use strict';
const kMaxLinesPerTick = 90;
const kShowOmittedLines = 10;
let lineCount, skippedCount;
let lastLine, lastStream;
let lineRepeat;
let skipTick;
let omittedLines;

function reset() {
	lineCount = 0;
	lastLine = undefined;
	lineRepeat = 0;
	skippedCount = 0;
	skipTick = Game.time
}

function printLine(stream, line) {
	if (stream === 0) {
		console.log(`<span style="white-space:normal">${line}</span>`);
	} else if (stream === 2) {
		console.log(`<span style="white-space:normal;color:#e79da7">${line}</span>`);
	} else {
		throw new Error(`Unknown output stream ${stream}`);
	}
}

function notifyRepeat(count, lastTick) {
	console.log(`<span style="color:#ffc66d">[Line repeated <b>${count}</b> time${count > 1 ? 's' : ''}${lastTick ? ' on previous tick' : ''}]</span>`);
}

module.exports = {
	print(stream, line) {
		line = `${line}`;
		if (lineCount < kMaxLinesPerTick) {
			if (line === lastLine && lastStream === stream) {
				++lineRepeat;
				return;
			} else if (lineRepeat > 0) {
				if (lineRepeat === 1) {
					printLine(stream, lastLine);
				} else {
					notifyRepeat(lineRepeat, false);
				}
				lastLine = undefined;
				lineRepeat = 0;
			}
		}
		if (++lineCount > kMaxLinesPerTick) {
			if (omittedLines === undefined) {
				omittedLines = [{ stream, line }];
			} else {
				omittedLines.push({ stream, line });
				if (omittedLines.length > kShowOmittedLines * 2) {
					omittedLines.splice(0, kShowOmittedLines);
				}
			}
			++skippedCount;
		} else {
			printLine(stream, line);
			lastLine = line;
			lastStream = stream;
		}
	},

	flush() {
		if (lineRepeat === 1) {
			printLine(lastStream, lastLine);
		} else if (lineRepeat > 0) {
			notifyRepeat(lineRepeat, skipTick !== Game.time);
		} else if (skippedCount > 0) {
			let showOmittedCount = Math.max(0, omittedLines.length - kShowOmittedLines);
			skippedCount -= showOmittedCount;
			if (skippedCount > 0) {
				console.log(`<span style="color:#ffc66d">[Omitted <b>${skippedCount}</b> line${skippedCount > 1 ? 's' : ''}${Game.time !== skipTick ? ' on previous tick' : ''}]</span>`);
			}
			for (let ii = Math.max(0, omittedLines.length - kShowOmittedLines); ii < omittedLines.length; ++ii) {
				printLine(omittedLines[ii].stream, omittedLines[ii].line);
			}
			if (Game.time !== skipTick) {
				console.log(`<span style="color:#ffc66d">[Output from current tick follows]</span>`);
			}
		}
		reset();
	}
};
