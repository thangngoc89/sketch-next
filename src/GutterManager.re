open Types;
open ExecutionState;

/*
 * This is a 2 pass process
 * 1st pass:
 *  Decide the parserState, create a list of execution state
 *  based on parser input, and lastExecutedLine
 * 2nd pass:
 *  Deciding the play status via mouse event/ cursor activity
 */

let makeParserState = (~phrs, ~state) => {
  let {maxLines, gutters: _, lastExecutedLine} = state;

  let arr = Array.make(maxLines, None);
  phrs
  |> List.iter(({startLine, endLine}) =>
       for (line in startLine to endLine) {
         if (line <= lastExecutedLine) {
           arr[line] = Some(Ps_executed);
         } else {
           arr[line] = Some(Ps_executable);
         };
       }
     );

  let lastExecutableLine = Js.Array.lastIndexOf(Some(Ps_executable), arr);

  arr
  |> Array.mapi((line, parserState) =>
       switch (parserState) {
       | None =>
         if (line <= lastExecutedLine) {
           Ps_executed;
         } else if (line <= lastExecutableLine) {
           Ps_executable;
         } else {
           Ps_non_executable;
         }
       | Some(ps) => ps
       }
     );
};

let patchCalculator = (before, after) => {
  let afLength = Array.length(after);
  let bfLength = Array.length(before);

  let ops = ref([]);

  let compareTheCommonPart = (~from, ~to_, ~before, ~after) =>
    for (i in to_ downto from) {
      if (before[i] != after[i]) {
        ops := [Patch_change(i, after[i]), ...ops^];
      };
    };

  if (afLength > bfLength) {
    for (i in bfLength to afLength - 1) {
      ops := [Patch_add(i, after[i]), ...ops^];
    };
    compareTheCommonPart(~from=0, ~to_=bfLength - 1, ~before, ~after);
  } else if (bfLength > afLength) {
    for (i in bfLength - 1 downto afLength) {
      ops := [Patch_remove(i), ...ops^];
    };
    compareTheCommonPart(~from=0, ~to_=afLength - 1, ~before, ~after);
  };
  ops^;
};

let calculateState = (~phrs, ~state) => {
  let parserState = makeParserState(~phrs, ~state);
  let {maxLines: _, gutters, lastExecutedLine} = state;

  let finalState =
    parserState
    |> Array.map(
         fun
         | Ps_executed => Executed
         | Ps_executable => Executable
         | Ps_non_executable => NonExecutable,
       );

  // Initial state, everything should be executable
  if (lastExecutedLine == (-1)) {
    let lastExecutableLine = Js.Array.lastIndexOf(Executable, finalState);
    finalState[lastExecutableLine] = ExecutableAndPlay;
  };

  ({...state, gutters: finalState}, patchCalculator(gutters, finalState));
};
