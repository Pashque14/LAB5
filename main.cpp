#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;
class AttackStrategy {
public:
    virtual void attack() const = 0;
    virtual ~AttackStrategy() = default;
};
class MissileAttack : public AttackStrategy {
public:
    void attack() const override {
        cout << "Запуск баллистических ракет!\n";
    }
};

class TorpedoAttack : public AttackStrategy {
public:
    void attack() const override {
        cout << "Пуск торпед!\n";
    }
};
class Sub {
protected:
    unique_ptr<AttackStrategy> attackStrategy;
public:
    virtual void describe() const = 0;
    void setAttackStrategy(unique_ptr<AttackStrategy> strategy) {
        attackStrategy = move(strategy);
    }
    void performAttack() const {
        if (attackStrategy) {
            attackStrategy->attack();
        } else {
            cout << "Атака не назначена!\n";
        }
    }
    virtual ~Sub() = default;
};
class NuclearSub : public Sub {
    int missiles;
public:
    NuclearSub(int m) : missiles(m) {}
    void describe() const override {
        cout << "Атомная подлодка с " << missiles << " ракетами\n";
    }
    int getMissiles() const { return missiles; }
};

class DieselSub : public Sub {
    int torpedoes;
public:
    DieselSub(int t) : torpedoes(t) {}
    void describe() const override {
        cout << "Дизельная подлодка с " << torpedoes << " торпедами\n";
    }
    int getTorpedoes() const { return torpedoes; }
};
class Mission {
public:
    void executeMission() {
        prepare();
        navigate();
        engage();
        retreat();
    }
    virtual ~Mission() = default;

protected:
    virtual void prepare() = 0;
    virtual void navigate() = 0;
    virtual void engage() = 0;
    virtual void retreat() = 0;
};
class CombatMission : public Mission {
protected:
    void prepare() override {
        cout << ">> Боевая миссия: подготовка оружия\n";
    }
    void navigate() override {
        cout << ">> Боевая миссия: выход на боевую позицию\n";
    }
    void engage() override {
        cout << ">> Боевая миссия: атака цели\n";
    }
    void retreat() override {
        cout << ">> Боевая миссия: отход на базу\n";
    }
};
class IIterator {
public:
    virtual Sub* next() = 0;
    virtual bool hasNext() const = 0;
    virtual ~IIterator() = default;
};
class IContainer {
public:
    virtual unique_ptr<IIterator> GetIterator() const = 0;
    virtual ~IContainer() = default;
};
class SubVector : public IContainer {
    vector<unique_ptr<Sub>> subs;
public:
    void add(unique_ptr<Sub> sub) {
        subs.push_back(move(sub));
    }
    Sub* get(size_t index) const {
        return subs[index].get();
    }
    size_t size() const {
        return subs.size();
    }
    unique_ptr<IIterator> GetIterator() const override;
};
class VectorIterator : public IIterator {
    const SubVector& container;
    size_t index = 0;
public:
    VectorIterator(const SubVector& c) : container(c) {}
    Sub* next() override {
        return container.get(index++);
    }
    bool hasNext() const override {
        return index < container.size();
    }
};
unique_ptr<IIterator> SubVector::GetIterator() const {
    return make_unique<VectorIterator>(*this);
}

class NuclearFilterIterator : public IIterator {
    unique_ptr<IIterator> it;
    Sub* nextSub = nullptr;
public:
    NuclearFilterIterator(unique_ptr<IIterator> iterator) : it(move(iterator)) {
        findNextNuclear();
    }
    void findNextNuclear() {
        nextSub = nullptr;
        while (it->hasNext()) {
            Sub* sub = it->next();
            if (dynamic_cast<NuclearSub*>(sub)) {
                nextSub = sub;
                break;
            }
        }
    }
    Sub* next() override {
        Sub* result = nextSub;
        findNextNuclear();
        return result;
    }
    bool hasNext() const override {
        return nextSub != nullptr;
    }
};
class WeaponCountIterator : public IIterator {
    unique_ptr<IIterator> it;
    int totalWeapons = 0;
public:
    WeaponCountIterator(unique_ptr<IIterator> iterator) : it(move(iterator)) {}
    Sub* next() override {
        if (!it->hasNext()) return nullptr;
        Sub* sub = it->next();
        if (auto nuclear = dynamic_cast<NuclearSub*>(sub)) {
            totalWeapons += nuclear->getMissiles();
        } else if (auto diesel = dynamic_cast<DieselSub*>(sub)) {
            totalWeapons += diesel->getTorpedoes();
        }
        return sub;
    }
    bool hasNext() const override {
        return it->hasNext();
    }
    int getTotalWeapons() const {
        return totalWeapons;
    }
};
class LimitIterator : public IIterator {
    unique_ptr<IIterator> it;
    int limit;
    int count = 0;
public:
    LimitIterator(unique_ptr<IIterator> iterator, int l) : it(move(iterator)), limit(l) {}
    Sub* next() override {
        if (count >= limit || !it->hasNext()) return nullptr;
        count++;
        return it->next();
    }
    bool hasNext() const override {
        return count < limit && it->hasNext();
    }
};
void demonstrateContainer(const IContainer& container) {
    cout << "Все подлодки:\n";
    auto it = container.GetIterator();
    while (it->hasNext()) {
        it->next()->describe();
    }
    cout << "\nТолько атомные подлодки:\n";
    auto nuclearIt = make_unique<NuclearFilterIterator>(container.GetIterator());
    while (nuclearIt->hasNext()) {
        nuclearIt->next()->describe();
    }
    cout << "\nПодсчет оружия:\n";
    auto weaponIt = make_unique<WeaponCountIterator>(container.GetIterator());
    while (weaponIt->hasNext()) {
        weaponIt->next()->describe();
    }
    cout << "Всего единиц оружия: " << weaponIt->getTotalWeapons() << endl;
    cout << "\nПервые 2 подлодки:\n";
    auto limitIt = make_unique<LimitIterator>(container.GetIterator(), 2);
    while (limitIt->hasNext()) {
        limitIt->next()->describe();
    }
}
void demonstrateAll(const SubVector& subs) {
    cout << "\n=== Демонстрация стратегий атаки ===\n";
    auto it = subs.GetIterator();
    while (it->hasNext()) {
        Sub* sub = it->next();
        sub->describe();
        sub->performAttack();
    }

    cout << "\n=== Демонстрация шаблонного метода ===\n";
    Mission* mission = new CombatMission();
    mission->executeMission();
    delete mission;
}
int main() {
    srand(time(0));
    setlocale(LC_ALL, "rus");
    SubVector subs;
    for (int i = 0; i < 3; i++) {
        auto nuclear = make_unique<NuclearSub>(rand() % 5 + 2);
        nuclear->setAttackStrategy(make_unique<MissileAttack>());
        subs.add(move(nuclear));

        auto diesel = make_unique<DieselSub>(rand() % 3 + 1);
        diesel->setAttackStrategy(make_unique<TorpedoAttack>());
        subs.add(move(diesel));
    }
    demonstrateAll(subs);
    demonstrateContainer(subs);

    return 0;
}
