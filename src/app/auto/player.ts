export enum Gender {
    MALE,
    FEMALE,
}

export interface Player {
    name: string;
    level: number;
    gender: Gender;
}