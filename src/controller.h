#pragma once
#include "zertz.h"

// Controller is a bridge between the game backend and the GUI.

struct Action { 
  std::optional<ObjIndex> src, dst; 
  void Push(ObjIndex idx) {
    assert(!(dst && !src));
    if (!src) {
      src = idx;
    } else {
      dst = idx;
    }
  }
  
  void Clear() {
    src = {};
    dst = {};
  }
  
  bool IsFull() {
    return src && dst;
  }
};

enum class ActionResult { kSelected, kSuccess, kFail };

struct ZController {
  ZController(std::shared_ptr<Zertz> zertz) : zertz(zertz) {}

  ActionResult OnClick(ObjIndex idx) {
    if (idx.type == ObjType::kUndoButton) {
      action.Clear();
      return UndoAction();
    }

    action.Push(idx);
    if (action.IsFull()) {
      auto res = ApplyAction();
      action.Clear();
      return res;
    } else {
      return ActionResult::kSelected;
    }
  }
  
 private:
  // Actual game logic below
  ActionResult ApplyAction() {
    switch(action.src->type) {
      case ObjType::kBall:
        return BallAction();
      case ObjType::kCell:
        return CellAction();
      case ObjType::kPile:
        return PileAction();
    }
  }
  
  ActionResult BallAction() {
    if (action.dst->type == ObjType::kCell) {
      return GetResult(zertz->MoveToBoard(action.src->GetBallId(), action.dst->GetQR()));
    } else if (action.dst->type == ObjType::kPile) {
      return GetResult(zertz->MoveToPile(action.src->GetBallId(), action.dst->GetPileId()));
    }
    return ActionResult::kFail;
  }

  ActionResult PileAction () {
    return ActionResult::kFail;
  }

  ActionResult CellAction() {
    if (action.dst->type != ObjType::kCell) {
      return ActionResult::kFail;
    }

    auto src_qr = *(action.src->qr);
    auto dst_qr = *(action.dst->qr);
    if (src_qr == dst_qr) {
      zertz->RemoveCell(src_qr);
      return ActionResult::kSuccess;
    }
    
    return ActionResult::kFail;
  }

  ActionResult UndoAction() {
    return GetResult(zertz->Undo());
  }

  ActionResult GetResult(bool is_success) {
    return is_success ? ActionResult::kSuccess : ActionResult::kFail;
  }

  std::shared_ptr<Zertz> zertz;
  Action action;
};