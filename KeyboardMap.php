#!/usr/bin/php
<?php

  // Copyright 2012 Artem Prilutskiy

  // Very useful link --> http://gunnarwrobel.de/wiki/Linux-and-the-keyboard.html
  // http://invisible-island.net/xterm/xterm.faq.html#how2_fkeys
  // http://www.shallowsky.com/linux/keymap.html
  // http://linux.die.net/man/5/keymaps
  // http://ftp.df.lth.se/%2F/pub/slackware/slackware-8.0/sgiboot/kernel-source/defkeymap.map

  function readTextFile($file)
  {
    if (!is_file($file))
    {
      print("File $file not found.\n");
      exit();
    }
    $text = file_get_contents($file);
    return explode("\n", $text);
  }

  function readGZipFile($file)
  {
    if (!is_file($file))
    {
      print("File $file not found.\n");
      exit();
    }
    $octets = file_get_contents($file);
    $text = gzinflate(substr($octets, 10, -8));
    return explode("\n", $text);
  }

  function readKeyCodes()
  {
    global $codes;

    // Set 2 scan code to Linux key code translation table
    // http://lxr.free-electrons.com/source/drivers/input/keyboard/atkbd.c
    $translations = array(
        0, 67, 65, 63, 61, 59, 60, 88,  0, 68, 66, 64, 62, 15, 41,117,
        0, 56, 42, 93, 29, 16,  2,  0,  0,  0, 44, 31, 30, 17,  3,  0,
        0, 46, 45, 32, 18,  5,  4, 95,  0, 57, 47, 33, 20, 19,  6,183,
        0, 49, 48, 35, 34, 21,  7,184,  0,  0, 50, 36, 22,  8,  9,185,
        0, 51, 37, 23, 24, 11, 10,  0,  0, 52, 53, 38, 39, 25, 12,  0,
        0, 89, 40,  0, 26, 13,  0,  0, 58, 54, 28, 27,  0, 43,  0, 85,
        0, 86, 91, 90, 92,  0, 14, 94,  0, 79,124, 75, 71,121,  0,  0,
       82, 83, 80, 76, 77, 72,  1, 69, 87, 78, 81, 74, 55, 73, 70, 99,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      217,100,255,  0, 97,165,  0,  0,156,  0,  0,  0,  0,  0,  0,125,
      173,114,  0,113,  0,  0,  0,126,128,  0,  0,140,  0,  0,  0,127,
      159,  0,115,  0,164,  0,  0,116,158,  0,172,166,  0,  0,  0,142,
      157,  0,  0,  0,  0,  0,  0,  0,155,  0, 98,  0,  0,163,  0,  0,
      226,  0,  0,  0,  0,  0,  0,  0,  0,255, 96,  0,  0,  0,143,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,107,  0,105,102,  0,  0,112,
      110,111,108,112,106,103,  0,119,  0,118,109,  0, 99,104,119,  0,
        0,  0,  0, 65, 99
    );

    $codes = array();

    // Make translation table to map Linux key code to KEYCODE_* macro that represents PS/2 scan code
    foreach (readTextFile("KeyCodes.h") as $line)
      if (preg_match("/^#define\\s+([\\w_]+)\\s+0x([0-9a-fA-F]+)/", $line, $matches))
      {
        $code = hexdec($matches[2]);
        // $code &= 0xffff;
        $index = ($code & 0x7f) | (($code & 0x80) << 1);
        if (($code & 0xff00) == 0xe000)
          $index |= 0x80;
        $key = $translations[$index];
        $codes[$key] = $matches[1];
      }

    // Not all codes presented in the codes dictionary
    for ($index = 0; $index < count($translations); $index ++)
      if (!array_key_exists($translations[$index], $codes))
      {
        $code = ($index & 0x7f) | (($index & 0x100) >> 1);
        if ($index & 0x80)
          $code |= 0xe000;
        $codes[$translations[$index]] = "0x" . sprintf("%02x", $code);
      }
  }

  function readSymbolMap()
  {
    global $symbols;

    // Initialize symbol table with some default values
    $symbols = array();

    // Read character map from X11 key defaults healder
    foreach (readTextFile("/usr/include/X11/keysymdef.h") as $line)
    {
      // Read Unicode symbol codes from comments of macro definitions
      if (preg_match("/^[#]define\\sXK[_]([\\w_]+)\\s.+\\sU[+]([0-9A-F]{4})\\s.+/", $line, $matches))
      {
        $key = $matches[1];
        $value = strtolower($matches[2]);
        if ((hexdec($value) >= 32) && (hexdec($value) <= 127))
          $value = "'" . str_replace("'", "\\'", str_replace("\\", "\\\\", chr(hexdec($value)))) . "'";
        else
          $value = "0x" . $value;
        $symbols[$key] = $value;
      }
      // Read special symbol codes from values of macro definitions
      if (preg_match("/^[#]define\\sXK[_]([\\w_]+)\\s+0xff([01][0-9a-z])/", $line, $matches))
      {
        $key = $matches[1];
        $value = "0x" . $matches[2];
        $symbols[$key] = $value;
      }
    }

    // Read sequences that have no modifiers from XTerm custom configuration file
    foreach (readTextFile("xterm.txt") as $line)
      if (preg_match("/^\\s*<Key>([\\w_]+)[:].+\\\"(.+)\\\"/", $line, $matches))
        $symbols[$matches[1]] = "\"\\033" . $matches[2] . "\"";

    // Make symbolic names for digits
    $digits = array("zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine");
    for ($index = 0; $index < count($digits); $index ++)
      $symbols[$digits[$index]] = "'" . $index . "'";
  }

  function decodeValue($code)
  {
    if (preg_match("/^(?:0x|Hex_)([0-9a-fA-F]+)$/", $code, $matches))
      return hexdec($matches[1]);
    if (preg_match("/^0(\\d+)$/", $code, $matches))
      return octdec($matches[1]);
    return intval($code);
  }

  function getSymbol($key, $page)
  {
    global $symbols;

    // Parse control code
    if (preg_match("/^Control_([\\w_]+)/", $key, $matches) &&
        array_key_exists($matches[1], $symbols))
    {
      $value = $symbols[$matches[1]];
      if (preg_match("/'(.)'/", $value, $matches))
        return "0x" . sprintf("%02x", ord(strtoupper($matches[1])) - 0x40);
      if (preg_match("/0x([0-9a-f]+)/", $value, $matches))
        return "0x" . sprintf("%02x", hexdec($matches[1]) - 0x40);
    }

    // Remove prefixes (meta characters, modifier of plus)
    if (preg_match("/^(?:Meta_|[+])(.*)/", $key, $matches))
      $key = $matches[1];

    // Check translation in dictionary
    if (array_key_exists($key, $symbols))
      return $symbols[$key];

    // Parse numeric value
    if (preg_match("/^(0x[0-9a-fA-F]{1,4}|Hex_[0-9A-F]|\\d{3,4})$/", $key))
      return "0x" . bin2hex(mb_convert_encoding(chr(decodeValue($key)), "UCS-2BE",  $page));

    // Remove unusable sequences
    if (preg_match("/(Show_.+|Scroll_.+|Console_.+|.+_Console|.+_Lock|Do|Boot|Help|Shift|Alt|AltGr|Control|Break|Compose|Ascii_.+|nul)/", $key))
      return 0;

    print("Warning! Unknown key definition: " . $key . "\n");
    return 0;
  }

  function convertModifiers($values)
  {
    $weights = array("shift" => 1, "altgr" => 2, "control" => 4, "alt" => 8, "shiftl" => 16, "shiftr" => 32, "ctrll" => 64, "ctrlr" => 128, "capsshift" => 256);
    $result = 0;
    foreach ($values as $value)
      $result |= $weights[strtolower($value)];
    return $result;
  }

  function readKeyboardMap($name, $page)
  {
    global $codes;
    global $symbols;

    $map = array();

    $lines = array_merge(
      readTextFile("defkeymap.map"),
      readGZipFile("/usr/share/keymaps/i386/qwerty/" . $name . ".kmap.gz"));

    // Read includes
    foreach ($lines as $line)
      if (preg_match("/^include\\s+\\\"([\\w_-]+)\\\"/", $line, $matches))
        $lines = array_merge($lines, readGZipFile("/usr/share/keymaps/i386/include/" . $matches[1] . ".inc.gz"));

    // Set translations defined in keymap file
    foreach ($lines as $line)
      if (preg_match("/^string\\s+(\\w+)\\s+[=]\\s+(\\\".*\\\")/", $line, $matches))
        $symbols[$matches[1]] = $matches[2];

    // Process key map
    foreach ($lines as $line)
      if (preg_match("/^\\s*((?:\\w+\\s+)+)?\\s*keycode\\s+(\\d+)\\s*[=]\\s*(.*)/", $line, $matches))
      {
        $key = $codes[decodeValue($matches[2])];

        if (empty($key))
        {
          print("Warning! Unknown key code: " . $matches[2] . "\n");
          continue;
        }

        $values = preg_split("/\\s+/", $matches[3], -1, PREG_SPLIT_NO_EMPTY);
        $modifiers = preg_split("/\\s+/", $matches[1], -1, PREG_SPLIT_NO_EMPTY);

        // Normalize values for keymaps that skips upper-case symbol definition (f.e. defkeymap, us)
        if ((count($values) == 1) && preg_match("/^[a-z]$/", $key))
          $values[] = strtoupper($values[0]);

        // Translate symbols
        for ($index = 0; $index < count($values); $index ++)
          $values[$index] = getSymbol($values[$index], $page);

        // Remove duplicates and empty values if possible
        if ((count($values) == 4) && ($values[0] == $values[2]) && ($values[1] == $values[3]))
          $values = array_slice($values, 0, 2);
        if (count($values) == 3)
          unset($values[2]);
        if ((count($values) == 2) && ($values[0] == $values[1]))
          unset($values[1]);
        for ($index = count($values) - 1; ($index >= 0) && ($values[$index] === 0); $index --)
          unset($values[$index]);

        // We should create unique key identity to override existing default values
        $identity = $key . "+" . convertModifiers($modifiers);

        // Remove any existing record with the same identity
        if (array_key_exists($identity, $map))
          unset($map[$identity]);

        // ... and override it if any values presented
        if (count($values) > 0)
          $map[$identity] = array("key" => $key, "modifiers" => $modifiers, "values" => $values);
      }

    return $map;
  }


  function buildHeaderFile($map)
  {
    $code = "// This file has been automatically generaded.\n";
    $code .= "// Please do not make changes.\n";
    $code .= "\n";
    $code .= "#include <avr/pgmspace.h>\n";
    $code .= "#include \"KeyboardMap.h\"\n";
    $code .= "\n";

    // Extract all string values to feet in Program EPROM
    $strings = array();
    foreach ($map as $key => $entry)
    {
      for ($index = 0; $index < count($entry["values"]); $index ++)
      {
        $value = $entry["values"][$index];
        if (substr($value, 0, 1) == "\"")
        {
          $name = array_search($value, $strings);
          if ($name === false)
          {
            $name = "keyString" . strval(count($strings));
            $code .= "char " . $name . "[] PROGMEM = " . $value . ";\n";
            $strings[$name] = $value;
          }
          $map[$key]["values"][$index] = $name;
        }
      }
    }

    $code .= "\n";
    $code .= "struct KeyMap keys[] PROGMEM =\n";
    $code .= "{\n";

    $lines = array();
    foreach ($map as $key => $entry)
    {
      // Open record, put key number
      $line = "  { " . $entry["key"] . ", ";

      // Build modifiers
      if (count($entry["modifiers"]) > 0)
        $line .= strtoupper("MODIFIER_" . implode(" | MODIFIER_", $entry["modifiers"]));
      else
        $line .= "0";

      // Build flags
      $flags = array();
      foreach ($entry["values"] as $value)
        if (preg_match("/^[a-z]/", $value))
          $flags[] = "FLAG_STRING" . strval(count($flags));
        else
          $flags[] = "FLAG_SYMBOL" .  strval(count($flags));
      $line .= ", " . implode(" | ", $flags);

      // Build values
      $line .= ", { " . implode(", ", $entry["values"]) . " }";

      // Close record, store line
      $line .= " }";
      $lines[] = $line;
    }

    $code .= implode(",\n", $lines) . ",\n";
    $code .= "};\n";
    $code .= "\n";
    $code .= "size_t keyCount = sizeof(keys) / sizeof(keys[0]);\n";
    $code .= "\n";

    return $code;
  }

  if (shell_exec("uname") != "Linux\n")
  {
    print("Please run this script under Linux.\n");
    exit();
  }

  readKeyCodes();
  readSymbolMap();

  // $map = readKeyboardMap("us", "ISO-8859-1");

  $map = readKeyboardMap("ru", "KOI8-R");
  file_put_contents("KeyboardMap.c", buildHeaderFile($map));

?>
