#ifndef PAGE_H_
#define PAGE_H_

#define PAGE_X    25
#define PAGE_Y    20
#define PAGE_W    270
#define PAGE_H    219
#define PAD       5
#define TEXT_L    (PAGE_X + PAD)
#define TEXT_R    (PAGE_X + PAGE_W - PAD)
#define LINE      10
#define HDR_Y0    25
#define HDR_Y1    35
#define RULE_HDR  42
#define BODY_Y    48
#define ROW(n)    (BODY_Y + (n) * LINE)
#define RULE_FOOT 214
#define FOOT_Y    220
#define COL0      TEXT_L
#define COL1      120
#define COL2      210

#endif
