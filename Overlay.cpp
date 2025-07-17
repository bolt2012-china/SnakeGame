#include "Overlay.h"
#include <stdexcept>

Overlay::Overlay(const sf::Vector2u& winSize,
                 const std::string&  text,
                 const std::string&  fontPath,
                 unsigned int        charSize,
                 const std::string&  audioPath,
                 sf::Color           maskColor)
: mMask   {sf::Vector2f(static_cast<float>(winSize.x),
                        static_cast<float>(winSize.y))},
  mFont   {},                                   // 1) 先构造 Font
  mText   {mFont, text, charSize},              // 2) 再用 Font 构造 Text ★:contentReference[oaicite:0]{index=0}
  mBuf    {}
{
    /* — 字体 — */
    if (!mFont.openFromFile(fontPath))
        throw std::runtime_error("Cannot open font: " + fontPath);

    mText.setFont(mFont);                       // 绑定已加载字体
    mText.setFillColor(sf::Color::White);

    const auto bounds = mText.getLocalBounds(); // FloatRect 现用 size 成员 ★
    mText.setOrigin({bounds.size.x * 0.5f,
                     bounds.size.y * 0.5f});
    mText.setPosition({winSize.x * 0.5f,
                       winSize.y * 0.5f});

    /* — 蒙版 — */
    mMask.setFillColor(maskColor);

    /* — 音效缓冲 — */
    mBuf.loadFromFile(audioPath);               // 失败则静默
}

void Overlay::play(sf::RenderWindow& window,
                   float             seconds,
                   const std::function<void()>& drawBelow)
{
    if (!window.isOpen()) return;

    // —— 首帧：先画底图，再叠加蒙版和提示文字 ——
    drawBelow();                 
    window.draw(mMask);
    window.draw(mText);
    window.display();

    // —— 播放音效 ——
    sf::Sound sound(mBuf);       // SFML-3：必须带 SoundBuffer 构造
    sound.play();

    
    sf::Clock timer;
    while (timer.getElapsedTime().asSeconds() < seconds && window.isOpen())
    {
        while (const auto ev = window.pollEvent())
            if (ev->is<sf::Event::Closed>()) {
                window.close();
                return;
            }

        sf::sleep(sf::milliseconds(10));  
    }
}
