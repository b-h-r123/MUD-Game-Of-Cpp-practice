#pragma once

#include <string>

// 打字机式剧情播放器。
//
// input     —— isFile 为 true 时是剧情文本文件路径，否则是直接文本内容。
// isFile    —— true 表示从文件读取剧情，false 表示直接使用 input。
// playerName—— 用于把文本中的 {{name}} 替换为玩家名字；为空则不替换。
// delayMs   —— 每个完整字符之间的停顿毫秒数。
//
// 播放期间按下空格可以跳过剩余全部内容，直接输出到结尾。
void printRhythm(const std::string& input, bool isFile, const std::string& playerName, int delayMs);
