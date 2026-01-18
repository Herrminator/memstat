@SETLOCAL
@SET _ALL=%*
@SET _ALL=%_ALL:-r=/r%
@CALL vcenv rc %_ALL%
@ENDLOCAL
