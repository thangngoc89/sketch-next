type pos = {
  char: int,
  line: int,
};

type loc = (pos, pos);

type phrase = {
  startLine: int,
  endLine: int,
  content: string,
  value: string,
};

type event = ..;
type event +=
  | UpdatePhrs(list(phrase))
  | EditedFromLine(int);

type patch('a) =
  | Patch_add(int, 'a)
  | Patch_change(int, 'a)
  | Patch_remove(int);

module ExecutionState = {
  type gutterState =
    | Executed /* don't show anything */
    | Executable /* show primary color */
    | ExecutableButIgnored /* when using mouse to hover on a gutter above */
    | NonExecutable /* show grey */
    | ExecutableAndPlay /* show a play button */
    | NonExecutableAndPlay; /* show a play button */

  type parserState =
    | Ps_executed
    | Ps_executable
    | Ps_non_executable;

  type gutterManager = {
    mutable maxLines: int,
    mutable lastExecutedLine: int,
    gutters: array(gutterState),
  };

  type gutterPatch = patch(gutterState);
};
