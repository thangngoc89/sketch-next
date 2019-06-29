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

module ExecutionState = {
  type lastExecutedLine = int;

  type lineGutterStatus =
    | Executed /* don't show anything */
    | Executable /* show primary color */
    | ExecutableButIgnored /* when using mouse to hover on a gutter above */
    | NonExecutable /* show grey */
    | CurrentLineExecutable /* show a play button */
    | CurrentLineNoneExecutable; /* show a play button */
};
