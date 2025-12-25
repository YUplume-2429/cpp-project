#pragma once
#include <QString>

// Image path placeholders (fill with actual paths or keep relative)
namespace Assets {
    // Spritesheets are expected to be 96x96 frames with 8 columns per row, 4 rows (N,E,S,W)
    // Replace with your actual file paths or keep names if placed next to the executable.
    static const QString PLAYER_SPRITESHEET = QStringLiteral(":/images/images/farmer walk.png");
    static const QString ZOMBIE_SPRITESHEET = QStringLiteral(":/images/images/zombie walk.png");
    static const QString HEALTH_IMAGE      = QStringLiteral(":/images/images/health.png");
}
