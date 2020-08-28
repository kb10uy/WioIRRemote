# WioIRRemote
Wio Terminal を色々リモコンにするやつ

## 機能
* Lua スクリプトでリモコン信号を生成・送信できる
    - NEC, AEHA フォーマットに対応

## lib ディレクトリについて
1. Lua 5.4 の src ディレクトリを適当に改名して入れ、以下の 2 ファイルを削除する
    - `lua.c`
    - `luac.c`
2. Seeed_Arduino_FS を展開し、ディレクトリが 1 つだけ残るように flatten する
3. Seeed_Arduino_SFUD を展開する

## Lua スクリプトの仕様
```lua
-- メニュー項目を生成するための配列テーブルを返す。
function getMenu()
    -- テーブルの配列テーブルとして構築する
    -- 各要素ごとに name (項目名、16bytes 以内) と kind (項目タイプ) は必須
    return {
        -- "button": 単一実行。 送信時にハイライトされている場合、この項目が true になる。
        {
            name = "foo",
            kind = "button",
        },

        -- "select": 複数項目切換。各 16bytes 以内で最大 8 項目まで選択項目を用意できる。
        {
            name = "bar",
            kind = "select",
            items = { "hoge", "huga", "piyo" },
        },

        -- "integer": 整数選択。最小値、最大値、ステップ値(省略可)を設定できる。
        {
            name = "bar",
            kind = "integer",
            minimum = 1,
            maximum = 10,
            step = 1,
        },
    };
end

-- 送信ボタンが押された際に呼び出される。
function onSend(ir, values)
    -- values には、メニュー項目名をキーとしてそれぞれに対応する値が格納されている。
    -- "button": メニュー項目がハイライトされた状態で送信した場合 true 、それ以外では false
    -- "select": メニュー項目で選択されている項目のインデックス (1-based)
    -- "integer": メニュー項目で表示されている値

    -- 信号の仕様を設定する。
    -- 1: NEC モード
    -- 2: AEHA モード
    -- 3: Sony モード
    ir:setType();

    -- カスタマーコードを設定する。
    -- NEC, AEHA モードのときのみ意味があり、 Sony モードでは使用しない。
    ir:setCustomerCode(0xDEAD);

    -- データを追加する。それぞれの値は 下位 8bits のみが考慮され、最大 256bytes まで蓄積される。
    -- 単一の値を渡すか、配列テーブルを渡す。
    ir:pushData(0x42);
    ir:pushData({ 0x42, 0x43 });

    -- 実際に送信処理を実行する。この関数の実行中に赤外線信号が送出される。
    ir:send();
end
```
