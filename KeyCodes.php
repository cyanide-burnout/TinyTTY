#!/usr/bin/php
<?php

  // Copyright 2012 Artem Prilutskiy

  $text = file_get_contents("keycodes.txt");

  $handle = fopen("KeyCodes.h", "c");
  fwrite($handle, "// This file has been automatically generaded.\n");
  fwrite($handle, "// Please do not make changes.\n");
  fwrite($handle, "\n");

  foreach (explode("\n", $text) as $line)
    if (preg_match("/^(.+)\\t(\\w*)\\t(?:(E0),)?(?:(E1),([0-9A-F]{2}),)?([0-9A-F]{2})\\t/", $line, $matches))
    {
      $name = "KEYCODE_" . str_replace(" ", "_", ((strlen($matches[2]) > 0) ? $matches[2] : $matches[1]));
      $value = hexdec(implode("", array_slice($matches, 3)));
      fwrite($handle, "#define " . $name . "\t0x" . sprintf("%06x", $value) . "UL\n");
    }

  fwrite($handle, "\n");
  fclose($handle);


?>
