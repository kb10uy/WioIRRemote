modeItems = { "Auto", "Warm", "Cool", "Dehumid." };
powerItems = { "Auto", "Low", "Medium", "High" };
powerValues = { 2, 3, 5, 7 };

function getMenu()
  return {
    { name = "turn_on", kind = "button" },
    { name = "turn_off", kind = "button" },
    { name = "mode", kind = "select", items = modeItems },
    { name = "temperature", kind = "integer", minimum = 18, maximum = 32 },
    { name = "power", kind = "select", items = powerItems },
  };
end

function onSend(ir, values)
  ir:setType(2);
  ir:setCustomerCode(0x5AAA);

  -- 定数部分
  local checksum = 0x2;

  -- ヘッダ
  ir:pushData({ 0x0C, 0x10 });

  -- 温度
  ir:pushData(values.temperature - 17);
  checksum = checksum ~ (values.temperature - 17);

  -- コマンド
  if values.turn_on then
    ir:pushData(0x11);
    checksum = checksum ~ 1;
  elseif values.turn_off then
    ir:pushData(0x21);
    checksum = checksum ~ 2;
  else
    ir:pushData(0x31);
    checksum = checksum ~ 3;
  end

  -- 風量・運転モード
  ir:pushData((powerValues[values.power] << 4) | (values.mode - 1));
  checksum = checksum ~ powerValues[values.power] ~ (values.mode - 1);

  -- タイマー、チェックサム
  ir:pushData({
    0x00, 0x08, 0x80,
    0x00, 0xF0,
    (checksum << 4) + 0x01,
  });

  ir:send();
end
