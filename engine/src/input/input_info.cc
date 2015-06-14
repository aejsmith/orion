/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Input information definitions.
 */

#include "core/hash_table.h"

#include "input/input_info.h"

#include <array>

/** Structure containing global input information. */
class GlobalInputInfo : public InputInfo {
public:
    /** Type of the global input information map. */
    typedef HashMap<std::string, InputInfo *> InfoMap;

    /** Type of the global input information array. */
    typedef std::array<InputInfo *, static_cast<size_t>(InputCode::kNumInputCodes)> InfoArray;
public:
    template <typename... Args>
    GlobalInputInfo(Args &&...args) :
        InputInfo(std::forward<Args>(args)...)
    {
        infoMap().insert(std::make_pair(this->name, this));
        infoArray()[static_cast<size_t>(this->code)] = this;
    }

    /** @return             Global input information map. */
    static InfoMap &infoMap() {
        static InfoMap map;
        return map;
    }

    /** @return             Global input information array. */
    static InfoArray &infoArray() {
        static InfoArray array;
        return array;
    }
};

/** Look up information for the given input code.
 * @param code          Code to look up.
 * @return              Pointer to info if code valid, null if not. */
const InputInfo *InputInfo::lookup(InputCode code) {
    return GlobalInputInfo::infoArray()[static_cast<size_t>(code)];
}

/** Look up information for the given named input.
 * @param name          Name to look up.
 * @return              Pointer to info if name known, null if not. */
const InputInfo *InputInfo::lookup(const char *name) {
    GlobalInputInfo::InfoMap map = GlobalInputInfo::infoMap();
    auto ret = map.find(name);
    return (ret != map.end()) ? ret->second : nullptr;
}

/*
 * We could declare this all as an array, but since C++ doesn't have designated
 * array initialisers (e.g. with GCC you can do [i] = ... in array initialisers)
 * we'd have to maintain the ordering of this list properly. Avoid that by
 * having the array populated at runtime in the constructors.
 */

static GlobalInputInfo infoA(InputCode::kA, "A", InputType::kButton);
static GlobalInputInfo infoB(InputCode::kB, "B", InputType::kButton);
static GlobalInputInfo infoC(InputCode::kC, "C", InputType::kButton);
static GlobalInputInfo infoD(InputCode::kD, "D", InputType::kButton);
static GlobalInputInfo infoE(InputCode::kE, "E", InputType::kButton);
static GlobalInputInfo infoF(InputCode::kF, "F", InputType::kButton);
static GlobalInputInfo infoG(InputCode::kG, "G", InputType::kButton);
static GlobalInputInfo infoH(InputCode::kH, "H", InputType::kButton);
static GlobalInputInfo infoI(InputCode::kI, "I", InputType::kButton);
static GlobalInputInfo infoJ(InputCode::kJ, "J", InputType::kButton);
static GlobalInputInfo infoK(InputCode::kK, "K", InputType::kButton);
static GlobalInputInfo infoL(InputCode::kL, "L", InputType::kButton);
static GlobalInputInfo infoM(InputCode::kM, "M", InputType::kButton);
static GlobalInputInfo infoN(InputCode::kN, "N", InputType::kButton);
static GlobalInputInfo infoO(InputCode::kO, "O", InputType::kButton);
static GlobalInputInfo infoP(InputCode::kP, "P", InputType::kButton);
static GlobalInputInfo infoQ(InputCode::kQ, "Q", InputType::kButton);
static GlobalInputInfo infoR(InputCode::kR, "R", InputType::kButton);
static GlobalInputInfo infoS(InputCode::kS, "S", InputType::kButton);
static GlobalInputInfo infoT(InputCode::kT, "T", InputType::kButton);
static GlobalInputInfo infoU(InputCode::kU, "U", InputType::kButton);
static GlobalInputInfo infoV(InputCode::kV, "V", InputType::kButton);
static GlobalInputInfo infoW(InputCode::kW, "W", InputType::kButton);
static GlobalInputInfo infoX(InputCode::kX, "X", InputType::kButton);
static GlobalInputInfo infoY(InputCode::kY, "Y", InputType::kButton);
static GlobalInputInfo infoZ(InputCode::kZ, "Z", InputType::kButton);
static GlobalInputInfo info1(InputCode::k1, "1", InputType::kButton);
static GlobalInputInfo info2(InputCode::k2, "2", InputType::kButton);
static GlobalInputInfo info3(InputCode::k3, "3", InputType::kButton);
static GlobalInputInfo info4(InputCode::k4, "4", InputType::kButton);
static GlobalInputInfo info5(InputCode::k5, "5", InputType::kButton);
static GlobalInputInfo info6(InputCode::k6, "6", InputType::kButton);
static GlobalInputInfo info7(InputCode::k7, "7", InputType::kButton);
static GlobalInputInfo info8(InputCode::k8, "8", InputType::kButton);
static GlobalInputInfo info9(InputCode::k9, "9", InputType::kButton);
static GlobalInputInfo info0(InputCode::k0, "0", InputType::kButton);
static GlobalInputInfo infoReturn(InputCode::kReturn, "Return", InputType::kButton);
static GlobalInputInfo infoEscape(InputCode::kEscape, "Escape", InputType::kButton);
static GlobalInputInfo infoBackspace(InputCode::kBackspace, "Backspace", InputType::kButton);
static GlobalInputInfo infoTab(InputCode::kTab, "Tab", InputType::kButton);
static GlobalInputInfo infoSpace(InputCode::kSpace, "Space", InputType::kButton);
static GlobalInputInfo infoMinus(InputCode::kMinus, "Minus", InputType::kButton);
static GlobalInputInfo infoEquals(InputCode::kEquals, "Equals", InputType::kButton);
static GlobalInputInfo infoLeftBracket(InputCode::kLeftBracket, "LeftBracket", InputType::kButton);
static GlobalInputInfo infoRightBracket(InputCode::kRightBracket, "RightBracket", InputType::kButton);
static GlobalInputInfo infoBackslash(InputCode::kBackslash, "Backslash", InputType::kButton);
static GlobalInputInfo infoSemicolon(InputCode::kSemicolon, "Semicolon", InputType::kButton);
static GlobalInputInfo infoApostrophe(InputCode::kApostrophe, "Apostrophe", InputType::kButton);
static GlobalInputInfo infoGrave(InputCode::kGrave, "Grave", InputType::kButton);
static GlobalInputInfo infoComma(InputCode::kComma, "Comma", InputType::kButton);
static GlobalInputInfo infoPeriod(InputCode::kPeriod, "Period", InputType::kButton);
static GlobalInputInfo infoSlash(InputCode::kSlash, "Slash", InputType::kButton);
static GlobalInputInfo infoCapsLock(InputCode::kCapsLock, "CapsLock", InputType::kButton);
static GlobalInputInfo infoF1(InputCode::kF1, "F1", InputType::kButton);
static GlobalInputInfo infoF2(InputCode::kF2, "F2", InputType::kButton);
static GlobalInputInfo infoF3(InputCode::kF3, "F3", InputType::kButton);
static GlobalInputInfo infoF4(InputCode::kF4, "F4", InputType::kButton);
static GlobalInputInfo infoF5(InputCode::kF5, "F5", InputType::kButton);
static GlobalInputInfo infoF6(InputCode::kF6, "F6", InputType::kButton);
static GlobalInputInfo infoF7(InputCode::kF7, "F7", InputType::kButton);
static GlobalInputInfo infoF8(InputCode::kF8, "F8", InputType::kButton);
static GlobalInputInfo infoF9(InputCode::kF9, "F9", InputType::kButton);
static GlobalInputInfo infoF10(InputCode::kF10, "F10", InputType::kButton);
static GlobalInputInfo infoF11(InputCode::kF11, "F11", InputType::kButton);
static GlobalInputInfo infoF12(InputCode::kF12, "F12", InputType::kButton);
static GlobalInputInfo infoPrintScreen(InputCode::kPrintScreen, "PrintScreen", InputType::kButton);
static GlobalInputInfo infoScrollLock(InputCode::kScrollLock, "ScrollLock", InputType::kButton);
static GlobalInputInfo infoPause(InputCode::kPause, "Pause", InputType::kButton);
static GlobalInputInfo infoInsert(InputCode::kInsert, "Insert", InputType::kButton);
static GlobalInputInfo infoHome(InputCode::kHome, "Home", InputType::kButton);
static GlobalInputInfo infoPageUp(InputCode::kPageUp, "PageUp", InputType::kButton);
static GlobalInputInfo infoDelete(InputCode::kDelete, "Delete", InputType::kButton);
static GlobalInputInfo infoEnd(InputCode::kEnd, "End", InputType::kButton);
static GlobalInputInfo infoPageDown(InputCode::kPageDown, "PageDown", InputType::kButton);
static GlobalInputInfo infoRight(InputCode::kRight, "Right", InputType::kButton);
static GlobalInputInfo infoLeft(InputCode::kLeft, "Left", InputType::kButton);
static GlobalInputInfo infoDown(InputCode::kDown, "Down", InputType::kButton);
static GlobalInputInfo infoUp(InputCode::kUp, "Up", InputType::kButton);
static GlobalInputInfo infoNumLock(InputCode::kNumLock, "NumLock", InputType::kButton);
static GlobalInputInfo infoKPDivide(InputCode::kKPDivide, "KPDivide", InputType::kButton);
static GlobalInputInfo infoKPMultiply(InputCode::kKPMultiply, "KPMultiply", InputType::kButton);
static GlobalInputInfo infoKPMinus(InputCode::kKPMinus, "KPMinus", InputType::kButton);
static GlobalInputInfo infoKPPlus(InputCode::kKPPlus, "KPPlus", InputType::kButton);
static GlobalInputInfo infoKPEnter(InputCode::kKPEnter, "KPEnter", InputType::kButton);
static GlobalInputInfo infoKP1(InputCode::kKP1, "KP1", InputType::kButton);
static GlobalInputInfo infoKP2(InputCode::kKP2, "KP2", InputType::kButton);
static GlobalInputInfo infoKP3(InputCode::kKP3, "KP3", InputType::kButton);
static GlobalInputInfo infoKP4(InputCode::kKP4, "KP4", InputType::kButton);
static GlobalInputInfo infoKP5(InputCode::kKP5, "KP5", InputType::kButton);
static GlobalInputInfo infoKP6(InputCode::kKP6, "KP6", InputType::kButton);
static GlobalInputInfo infoKP7(InputCode::kKP7, "KP7", InputType::kButton);
static GlobalInputInfo infoKP8(InputCode::kKP8, "KP8", InputType::kButton);
static GlobalInputInfo infoKP9(InputCode::kKP9, "KP9", InputType::kButton);
static GlobalInputInfo infoKP0(InputCode::kKP0, "KP0", InputType::kButton);
static GlobalInputInfo infoKPPeriod(InputCode::kKPPeriod, "KPPeriod", InputType::kButton);
static GlobalInputInfo infoNonUSBackslash(InputCode::kNonUSBackslash, "NonUSBackslash", InputType::kButton);
static GlobalInputInfo infoApplication(InputCode::kApplication, "Application", InputType::kButton);
static GlobalInputInfo infoKPEquals(InputCode::kKPEquals, "KPEquals", InputType::kButton);
static GlobalInputInfo infoLeftCtrl(InputCode::kLeftCtrl, "LeftCtrl", InputType::kButton);
static GlobalInputInfo infoLeftShift(InputCode::kLeftShift, "LeftShift", InputType::kButton);
static GlobalInputInfo infoLeftAlt(InputCode::kLeftAlt, "LeftAlt", InputType::kButton);
static GlobalInputInfo infoLeftSuper(InputCode::kLeftSuper, "LeftSuper", InputType::kButton);
static GlobalInputInfo infoRightCtrl(InputCode::kRightCtrl, "RightCtrl", InputType::kButton);
static GlobalInputInfo infoRightShift(InputCode::kRightShift, "RightShift", InputType::kButton);
static GlobalInputInfo infoRightAlt(InputCode::kRightAlt, "RightAlt", InputType::kButton);
static GlobalInputInfo infoRightSuper(InputCode::kRightSuper, "RightSuper", InputType::kButton);
static GlobalInputInfo infoMouseX(InputCode::kMouseX, "MouseX", InputType::kAxis);
static GlobalInputInfo infoMouseY(InputCode::kMouseY, "MouseY", InputType::kAxis);
static GlobalInputInfo infoMouseScroll(InputCode::kMouseScroll, "MouseScroll", InputType::kAxis);
static GlobalInputInfo infoMouseLeft(InputCode::kMouseLeft, "MouseLeft", InputType::kButton);
static GlobalInputInfo infoMouseRight(InputCode::kMouseRight, "MouseRight", InputType::kButton);
static GlobalInputInfo infoMouseMiddle(InputCode::kMouseMiddle, "MouseMiddle", InputType::kButton);
