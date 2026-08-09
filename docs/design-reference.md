# PROOF OF FEELING — DESIGN REFERENCE

**Affective Core AC-7** · Speculative Design / Mechatronic Artefact
Author: Vincent Santosa

---

## 1. Project Context & Story

*Proof of Feeling* is a speculative artefact from a near-future in which
humanoid robots have become convincing enough at simulating emotion that
human trust in them collapses. Regulators respond the way institutions
usually respond to a trust crisis: not by earning trust, but by
mandating proof of it. Every advanced humanoid must now contain an
**Affective Core** — a commercially manufactured robotic heart, sold
under the brand **AC-7** by *Companion Systems Inc.*, that makes a
robot's internal state externally visible through pulse, motion, and
light.

The heart does not sustain the robot. It sustains the *human's belief*
in the robot. That distinction is the entire design brief: this is not a
life-support organ, it's a trust interface wearing the shape of one —
performative rather than biological, engineered to be watched rather
than to function.

**Why this framing matters:** the project's actual subject isn't
robotics — it's what happens when trust is demanded on command instead
of earned. Manufactured proof of an inner state is not the same thing as
the inner state itself; it's a performance authored by whoever controls
the evidence. In the world of the AC-7, that's Companion Systems Inc. —
they design the pulse curves, choose the light behaviour, and program
every interaction mode in advance. The robot never gets to generate its
own evidence of feeling; the corporation does it on the robot's behalf,
the same way a regulator's compliance certificate stands in for the
thing it certifies. The heart is affirmative *within* the fiction (a
branded, reassuring, compliant product) while functioning as *critical*
design when offered to a viewer — it deliberately declines to answer
whether visible emotion equals genuine emotion, or who actually benefits
once a feeling becomes measurable and sellable.

This is also where the project is personal rather than purely
speculative. It's shaped by a Javanese sense that humility and quiet
consistency earn trust, while insistence on being seen and believed
usually signals the opposite — a relationship (or a system) that doesn't
trust itself enough to go unwatched. The AC-7 inverts that: it rewards
spectacle over restraint, and treats legibility as though it were the
same thing as trustworthiness. The project doesn't resolve that tension.
It's built to make a viewer sit inside it.

### The critical question

Not "can robots have feelings?" but:

**What happens when emotion becomes a manufactured interface?**

— and, closer to the surface: can empathy be engineered? Can trust be
designed? Is visible suffering required for moral consideration? Does
emotional transparency create understanding, or control? Who benefits
when emotion becomes measurable?

---

## 2. The User Side — Experiencing the Physical Prototype

The prototype is displayed as a **removed organ undergoing inspection**,
not a sculpture — the audience should feel they're looking at a real
industrial artefact, complete with product labels, serial numbers, and
inspection warnings ("Trust calibration required every 500 operating
hours."), sitting in a display cradle labelled `AFFECTIVE CORE · MODEL
AC-7 · PROPERTY OF COMPANION SYSTEMS INC.`

There is no button to press, no explicit "start" — the object is always
performing, the same way the fiction insists a real Affective Core would
be. What a viewer actually experiences:

* **It's always alive.** Two servo-driven anatomical structures move
  continuously — a slow, sweeping contraction/twist, mirrored between
  the two, so the object never looks static or off.
* **A pulse of light chases through the shell**, top to bottom, through
  a row of exposed "pulse point" LEDs behind the shell's translucent
  cutouts — read as bioelectric or vein-like activity rather than a
  simple blink.
* **It notices you.** An ultrasonic sensor detects when a viewer leans
  in. As they get closer, both the light pulse and the servo motion
  speed up — continuously, not as a discrete jump — so the object reads
  as *reacting to attention*, the same way a heart rate rises under
  scrutiny. Stepping back lets it slow back down.
* **It has an eye.** A small embedded screen shows a single eye that
  blinks at irregular, unscripted-feeling intervals — the one part of
  the object that behaves less like an instrument and more like it's
  looking back.
* **It carries the evidence of inspection**, not concealment: exposed
  copper traces styled after blood vessels and neural pathways, visible
  test points, fasteners, and calibration tags — synthetic and organic
  and corporate registers layered on top of each other on purpose.

The intended effect isn't "this looks alive" — it's closer to "this
looks like it was *designed to convince me* it's alive," and leaving
that gap visible is the point. A viewer who leans in and watches the
object visibly quicken in response is, in that moment, doing exactly
what the fiction's regulators designed the object for: deciding whether
to trust a machine by watching it perform. The project succeeds if that
prompts the discomfort of "would I actually trust something more because
it looks vulnerable to me?" — and fails if the only questions it
provokes are about the engineering (*what servo is that, does the
sensor work*) rather than the premise.

---

## Appendix: Physical Envelope

Target: 300mm × 250mm × 200mm · Maximum: 400mm × 400mm × 400mm (course
compliance).

For hardware specifics — servo/sensor/LED/display wiring, GPIO
assignments, firmware behaviour — see
[`pcb-design-reference.md`](pcb-design-reference.md) and
[`../firmware/README.md`](../firmware/README.md).
