#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>                // pollEvent()
#include <functional>
#include <string>

/**
 * 半透明蒙版 + 文字 + 音效，一次性播放。
 */
class Overlay {
public:
    Overlay(const sf::Vector2u& winSize,
            const std::string&  text,
            const std::string&  fontPath,
            unsigned int        charSize,
            const std::string&  audioPath,
            sf::Color           mask = {120, 120, 120, 180});

    /// 阻塞 seconds 秒；期间只处理窗口关闭事件
    void play(sf::RenderWindow& window,
              float             seconds,
              const std::function<void()>& drawBelow);

private:
    sf::RectangleShape mMask;
    sf::Font           mFont;
    sf::Text           mText;      // ➜ Text 无默认 ctor，见下
    sf::SoundBuffer    mBuf;       // 声音缓冲
};
