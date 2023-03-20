#ifndef PLANT_H
#define PLANT_H
#define CC_CONSTRUCTOR_ACCESS public

#include <SDL.h>

enum class PlantsType
{
    None = -1,
    SunFlower,
    PeaShooter,
    WallNut,
    CherryBomb,
    PotatoMine,
    CabbagePult,
    Torchwood,
    Spikeweed,
    Garlic,
    FirePeaShooter,
    Jalapeno,
    AcidLemonShooter,
    Citron
};

class Plants
{
public:
    /* *
     * Pause all plant actions
     */
    static void stopPlantsAllAction();

    // /* *
    //  * Create pictures
    //  */
    // virtual Sprite *createPlantImage() = 0;

    // /* *
    //  * Create plant animations
    //  */
    // virtual void createPlantAnimation() = 0;

    /* *
     *Judging the relative position between zombies and plants
     */
    virtual void determineRelativePositionPlantsAndZombies();

    /* *
     * Detection of plant life
     */
    virtual void checkPlantHealthPoint() {}

    // /* *
    //  * set node
    //  */
    // virtual void setPlantNode(Node *node) { _node = node; }

    /* *
     * set location
     */
    virtual void setPlantPosition(const Vec2 &position) { _position = position; }

    /* *
     * Set the drawing order
     */
    virtual void setPlantLocalZOrder(const int &order) { _zOrder = order; }

    /* *
     * Set the ranks of plants
     */
    virtual void setPlantRowAndColumn(const Vec2 &rowAndColumn) { _rowAndColumn = rowAndColumn; }

    /* *
     * Set whether the plant is visible
     */
    virtual void setPlantVisible(const bool visible);

    /* *
     * get plant type
     */
    PlantsType getPlantType() const { return _plantsType; }

    /* *
     * Set plant markers
     */
    virtual void setPlantTag(const int &tag) { _plantTag = tag; }

    /* *
     * Set plant health
     */
    virtual void setPlantHealthPoint(int healthPoint) { _healthPoint = healthPoint; }

    /* *
     * Set plant size
     */
    virtual void setPlantScale();

    /* *
     * Get plant markers
     */
    virtual int getPlantTag() const { return _plantTag; }

    /* *
     * get animation
     */
    SkeletonAnimation *getPlantAnimation() { return _plantAnimation; }

    /* *
     *Determine whether the plant is alive or not
     */
    virtual bool getPlantIsSurvive() const;

    /* *
     * Get plant health
     */
    virtual float getPlantHealthPoint() const { return _healthPoint; }

    /* *
     *@ 1. Get the row and column where the plant is located
     */
    virtual Vec2 getPlantRowAndColumn() const { return _rowAndColumn; }

    /* *
     *@ 2. Get the row where the plant is located
     */
    virtual int getPlantRow() const { return _rowAndColumn.x; }

    /* *
     *@ 3. Get the column where the plant is located
     */
    virtual int getPlantColumn() const { return _rowAndColumn.y; }

    /* *
     * Get whether to display
     */
    virtual bool getPlantVisible();

    /* *
     * Get whether the plant can be deleted
     */
    virtual bool *getPlantIsCanDelete();

    CC_CONSTRUCTOR_ACCESS :

        Plants(Node *node = nullptr, const Vec2 &position = Vec2::ZERO);
    ~Plants();

protected:
    /* *
     *grow plants
     */
    virtual SkeletonAnimation *plantInit(const std::string &plantname, const std::string &animaionname);

    /* *
     * create plant pictures
     */
    virtual void imageInit(const std::string &name, const Vec2 &position);

    /* *
     *Reduces plant health
     */
    virtual void reducePlantHealthPoint(int number) { _healthPoint -= number; }

    /* *
     * Mud splash animation
     */
    virtual void setPlantSoilSplashAnimation(const float &scale);

    /* *
     *Set plant shadow
     */
    virtual void setPlantShadow(const float &scale);

    /* *
     * Set plants to flash when taking damage
     */
    virtual void setPlantHurtBlink();
    virtual void setPlantHurtBlink(PlantsType type) const;

    /* *
     * Get whether the zombie is in front of the plant
     */
    virtual bool getZombieIsTheFrontOfPlant(Zombies *zombie) const;

    /* *
     * Get whether the zombie is in the same row as the plant
     */
    virtual bool getZombieIsSameLineWithPlant(Zombies *zombie) const;

    /* *
     * Get whether the zombie has encountered a plant
     */
    virtual bool getzombieIsEncounterPlant(Zombies *zombie) const;

    /* *
     * Zombies eat plants
     */
    virtual void zombieEatPlant(Zombies *zombie);

    /* *
     * Zombies recover from eating plants
     */
    virtual void zombieRecoveryMove(Zombies *zombie);

private:
    void setPlantGLProgram();

protected:
    int _sunNeed;                            // Sunshine needs
    int _costGold;                           // spend gold coins
    int _costMasonry;                        // cost masonry
    int _combatEffectiveness;                // combat effectiveness
    int _plantTag;                           // plant tag
    int _zOrder;                             // drawing order
    int _plantNumber;                        // store the plant number (unique)
    bool _isLoop;                            // Whether to loop
    bool _isCanDelete[2];                    // Can delete
    float _coolDownTime;                     // cooling time
    float _healthPoint;                      // life value
    float _totalHealthPoint;                 // Total value of life
    Vec2 _position;                          // position
    Vec2 _rowAndColumn;                      // Detailed address, row and column
    SkeletonAnimation *_plantAnimation;      // plant animation
    Node *_node;                             // node
    Global *_global;                         // global variable
    Sprite *_plantImage;                     // plant image
    Sprite *_plantShadow;                    // plant shadow
    PlantsType _plantsType;                  // plant type
    SkeletonAnimation *_soilSplashAnimation; // soil splash animation
    static int plantNumber;                  // plant number (unique)

private:
    static GLProgram *_normalGLProgram;
    static GLProgram *_highLightGLProgram;
    GLProgramState *_highLightGLProgramState;
    float _highLightIntensity;
};

#endif
