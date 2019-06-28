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
  | UpdatePhrs(list(phrase));

type state = {mutable lastExecLine: int};
