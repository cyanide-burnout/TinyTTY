#!/usr/bin/php
<?php

  // Copyright 2012 Artem Prilutskiy

  $content = file_get_contents("CharacterMap.txt");

  $code = array();
  foreach (explode("\n", $content) as $line)
  {
    $parameters = explode(" = ", $line);
    if (count($parameters) == 2)
    {
      $key = bin2hex(mb_convert_encoding($parameters[0], "UCS-2BE", "UTF-8"));
      $value = $parameters[1];
      $code[] = "  { 0x$key, $value }";
    }
  }

  $handle = fopen("CharacterMap.c", "c");
  fwrite($handle, "// This file has been automatically generaded.\n");
  fwrite($handle, "// Please do not make changes.\n");
  fwrite($handle, "\n");
  fwrite($handle, "#include \"CharacterMap.h\"\n");
  fwrite($handle, "#include <avr/pgmspace.h>\n");
  fwrite($handle, "\n");
  fwrite($handle, "struct CharacterMap characters[] PROGMEM =\n");
  fwrite($handle, "{\n");
  fwrite($handle, implode(",\n", $code) . "\n");
  fwrite($handle, "};\n");
  fwrite($handle, "\n");
  fwrite($handle, "size_t characterCount = sizeof(characters) / sizeof(characters[0]);\n");
  fwrite($handle, "\n");
  fclose($handle);

?>
