#pragma once

inline short SIMULATE_PACKET_LOSS = 1;
inline short LOSS_PROBABILITY = 30;
inline short LOSS_ACK_PROBABILITY = 10;
inline short DROP_EVERY_NTH = 0;
inline short DROP_ACK_EVERY_NTH = 0;
inline short STATS_INTERVAL = 5000;

inline unsigned long lastStatsTime = 0;
inline unsigned long totalSent = 0;
inline unsigned long totalAcks = 0;
inline unsigned long totalRetries = 0;
inline unsigned long totalFailed = 0;