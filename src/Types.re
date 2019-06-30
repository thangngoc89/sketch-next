module MutableIntSet = Belt.MutableSet.Int;

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
  | EditedFromLine(int)
  | CursorMove(int);

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

  let string_of_gutterState =
    fun
    | Executed => "executed" /* don't show anything */
    | Executable => "executable" /* show primary color */
    | ExecutableButIgnored => "executable but ignored" /* when using mouse to hover on a gutter above */
    | NonExecutable => "non executable" /* show grey */
    | ExecutableAndPlay => "executable and play" /* show a play button */
    | NonExecutableAndPlay => "none executable and play"; /* show a play button */
  type parserState =
    | Ps_executed
    | Ps_executable
    | Ps_non_executable;

  type gutterManager = {
    mutable phrs: list(phrase),
    mutable maxLines: int,
    mutable lastExecutedLine: int,
    mutable requestToExecuteAtLine: int,
    mutable playableLines: MutableIntSet.t,
    gutters: array(gutterState),
  };

  type gutterEvent =
    | Ge_request_to_execute_at_line(int);

  type gutterPatch = patch(gutterState);
};
